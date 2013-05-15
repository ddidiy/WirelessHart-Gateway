#!/bin/sh

#this script is used to make various checks on the system

#used by all reports:
UTILS_FILE="/tmp/utils.txt"
UTILS_FILE_TMP="/tmp/utils.tmp"
UTILS_RC="/tmp/.utils.rc"

#used by login detection
LOGIN_VAR="LastLoginReport"
MSG_FILE="/var/log/messages"
MSG_FILE_TMP="/tmp/messages.tmp"
MSG_FILE_SIZE_LIMIT=30000	#bytes

#used by flash space detection
FLASH_VAR="LastFlashReport"
FLASH_DFLT_LIMIT=1024	#KB

#used by memory low detection
. common.sh
MEMORY_VAR="LastMemoryReport"
MEMORY_DFLT_LIMIT=4	#MB: this is real limit, I will not customize this

#used by temp detection
TEMP_LOW_VAR="LastTempLowReport"
TEMP_HIGH_VAR="LastTempHighReport"
TEMP_LOW_DFLT_LIMIT=1		#Fahrenheit, should allow negative
TEMP_HIGH_DFLT_LIMIT=100	#Fahrenheit,

#used by humidity detection
HUMIDITY_VAR="LastHumidityReport"
HUMIDITY_DFLT_LIMIT=60

#used by BATTERY detection
BATTERY_VAR="LastBatteryReport"
BATTERY_DFLT_LIMIT=10000

#used by NTP to determine if it's time to set RTC time. 3600: at least 3600 sec between two succesive RTC set
TIME_RTC_VAR="LastRTCSet"
TIME_RTC_DFLT_LIMIT=3600

#used by configuration changed detection
CONFIG_CHANGED_TMP="/tmp/config_changed.tmp"

#create /tmp/.utils.rc file - with env variables- from config.ini, if necessary
#load environ variables
load_env()
{
	[ -f $UTILS_RC -a $UTILS_RC -nt ${NIVIS_PROFILE}config.ini ] && . $UTILS_RC && return 1
	sed -n "s/ *FLASH_LOW_TRESHOLD *= *\([0-9]*\).*/NIVIS_FLASH_LOW=\1/pI
			s/ *TEMPERATURE_HIGH *= *\([0-9\-]*\).*/NIVIS_TEMP_HIGH=\1/pI
			s/ *TEMPERATURE_LOW *= *\([0-9\-]*\).*/NIVIS_TEMP_LOW=\1/pI
			s/ *HUMIDITY_HIGH *= *\([0-9]*\).*/NIVIS_HUMIDITY_HIGH=\1/pI
			s/ *BATTERY_LOW *= *\([0-9]*\).*/NIVIS_BATTERY_LOW=\1/pI" ${NIVIS_PROFILE}config.ini > $UTILS_RC
	. $UTILS_RC
}

# Truncate a file if its size exceeded MAX_FILE_SIZE_LIMIT
# $1 file to truncate
# $2 optional parameter to execute after the file was truncated
truncate_file()
{
	FILE=$1
	msg_file_size=`GetFileSize $FILE`
	if [ $msg_file_size -gt $MSG_FILE_SIZE_LIMIT ] ; then
		cat /dev/null > $FILE
		log2flash "The file $FILE exceeded $MSG_FILE_SIZE_LIMIT, truncated"
		if [ -n "$2" ]; then
			$2
		fi
	fi
}


#check last login, generate events in for each new login detected
check_login()
{
	grep login $MSG_FILE > $MSG_FILE_TMP

	truncate_file $MSG_FILE
	truncate_file "/var/log/http.log" "http_ctl.sh restart"
	truncate_file "/var/log/https.log" "http_ctl.sh restart"

	total=`wc -l < $MSG_FILE_TMP`
	last_time_reported=`sed -n "s/${LOGIN_VAR}=//p" ${UTILS_FILE}`
	[ -z "${last_time_reported}" ] && add_var ${LOGIN_VAR} "" && update_var ${LOGIN_VAR} "never"

	i=$total
	login_time=1

	while [ $i -ge 1 ] ;
	do     #here a balance - use head for i < $total/2 and tail for the rest
		login_line=`head -n $i $MSG_FILE_TMP | tail -n 1`
		login_time=`echo "$login_line" | cut -c -15`
		[ "$login_time" = "$last_time_reported" ] && rm $MSG_FILE_TMP && return 0
		[ $i -eq $total ] && update_var ${LOGIN_VAR} "$login_time"
		login_successfull=`echo "$login_line" | grep "root login" | wc -l`
		if [ $login_successfull = 1 ]; then
			[ -x raise_an_ev ] && raise_an_ev -e 9 -v $login_successfull		#Local Login ok
			log2flash "Login OK at $login_time"
		else
			[ -x raise_an_ev ] && raise_an_ev -e 1 #invalid security on AiNode level (login failed)
			log2flash "Login FAILED at $login_time"
		fi
		i=$(($i-1));
	done
	rm $MSG_FILE_TMP
}

#check profile files mtime against profile folder mtime,
#generate event if any profile file is newer than the profile folder
check_config_changed()
{
	rm -f $CONFIG_CHANGED_TMP

	for i in `ls $NIVIS_PROFILE` ; do
		[ $NIVIS_PROFILE$i -nt $NIVIS_PROFILE ] && ls -la $NIVIS_PROFILE$i >> $CONFIG_CHANGED_TMP
	done

	if [ -f $CONFIG_CHANGED_TMP ]; then
		#save profile time too, for reference
		ls -lad $NIVIS_PROFILE >> $CONFIG_CHANGED_TMP
		log2flash "Configuration change detected"
		ra_cmd.sh 35 3 $CONFIG_CHANGED_TMP && touch $NIVIS_PROFILE
		rm -f $CONFIG_CHANGED_TMP
	fi
}

#check available memory
check_free_memory()
{
	MemoryStatus
	memAvailable=$?
	#no customisable memory limit, it too important...
	check $memAvailable "-lt" ${MEMORY_DFLT_LIMIT} $MEMORY_VAR 2 30 "Memory free" "MB"
}

#check flash size, generate event if drops below some limit
check_flash()
{
	flash=`df | grep mtdblock2 | cut -c 41-50`
	check $flash "-lt" ${NIVIS_FLASH_LOW:-$FLASH_DFLT_LIMIT} $FLASH_VAR 13 31 "Flash space" "KB"
	while [ $flash -lt $FLASH_DFLT_LIMIT ]; do
		RM_CANDIDATE=`ls -1 ${NIVIS_ACTIVITY_FILES}/work_time.debug.* | tail -n 1`
		[ -z "$RM_CANDIDATE" ] && return
		rm $RM_CANDIDATE
		log2flash "Erased $RM_CANDIDATE"
		flash=`df | grep mtdblock2 | cut -c 41-50`
	done
}

#check humidity and temperature, generate event if limits are exceeded
check_temp_humid()
{
	text=`i2c-temp-humid -a --silent`
	temp=`echo $text | cut -d' ' -f1 | cut -d'.' -f1`	#get rid of decimals
	humidity=`echo $text | cut -d' ' -f2 | cut -d'.' -f1` #get rid of decimals
	check $temp     "-lt" ${NIVIS_TEMP_LOW:-$TEMP_LOW_DFLT_LIMIT}      $TEMP_LOW_VAR  17 32 "Temperature" "F"
	check $temp     "-gt" ${NIVIS_TEMP_HIGH:-$TEMP_HIGH_DFLT_LIMIT}    $TEMP_HIGH_VAR 16 32 "Temperature" "F"
	check $humidity "-gt" ${NIVIS_HUMIDITY_HIGH:-$HUMIDITY_DFLT_LIMIT} $HUMIDITY_VAR  18 33 "Humidity"    "%%"
}

#check battery, generate event if limits are exceeded
check_battery()
{
	battery=`i2c-adc -c 1 -p 12 -t s --silent`
	check $battery "-lt" ${NIVIS_BATTERY_LOW:-$BATTERY_DFLT_LIMIT} $BATTERY_VAR 19 34 "Battery" "mV"
}

remove_var()
{
	grep -v $1 ${UTILS_FILE} > ${UTILS_FILE_TMP} &&	mv ${UTILS_FILE_TMP} ${UTILS_FILE}
}

add_var()
{
	echo "$1=$2" >> ${UTILS_FILE}
}

update_var()
{	#sort | uniq to avoid unicity problems
	sed "s/$1=.*/$1=$2/" ${UTILS_FILE} | sort | uniq > ${UTILS_FILE_TMP} &&\
		mv ${UTILS_FILE_TMP} ${UTILS_FILE}
}

check()
{
	value=$1		#value to check							Example 9652
	cmp=$2			#comparison: 							Example "-lt" or "-gt"
	limit=$3		#limit to check against					Example 1024
	varname=$4		#variable name to hold last report time	Example LastLoginReport
	event_bad=$5	#event number to raise for val BAD		Example 19
	event_good=$6	#event number to raise for val OK		Example 34
	devname=$7		#device name							Example Battery
	um=$8			#measurement unit for this device		Example mV

	latest=`sed -n "s/${varname}=//p" ${UTILS_FILE}`
	[ ${cmp} = "-lt" ] && hilo="LOW" || hilo="HIGH"

	if [ ${value} ${cmp} ${limit} ]; then
		#value low
		if [ -z ${latest} ]; then
			[ -x raise_an_ev ] && raise_an_ev -e ${event_bad} -v ${value}
			log2flash ${devname}" "${value}" "${um}" "${hilo}". Limit ${limit} "$um
			add_var ${varname} `date +%s`
		fi
	else
		#value ok
		if [ "${latest}" ]; then
			diff=$((`date +%s`-${latest}))
			raise_an_ev -e ${event_good} -v ${value}
			log2flash ${devname}" "${value}" "${um}" OK (was ${hilo} ${diff}s). Limit ${limit} "$um
			remove_var ${varname}
		fi
	fi
}

check_time()
{
	ntpadjust.sh affect_ntpd
}

check_udhcpc()
{
	if [ -f /access_node/etc/config/no_dhcp ]; then
		return # DHCP client disabled, do not attempt to restart
	fi

	if [ $((`pidof udhcpc | wc -w`))  -lt 1 ]; then
		log2flash "WARN: udhcpc not running. Trying to start it."
		/usr/sbin/udhcpc -b -s /access_node/etc/udhcpc/udhcpc.sh -i eth0:0
	fi
}

check_all()
{
#check for instances of:
#       utils.sh, activate.sh, LGFirmware.tar.sh LGProfile.tgz.sh
# and don't run if any of those is already running,
# in order to reduce the stress level
	PIDS=`pidof utils.sh activate.sh LG*sh`
	stress_no=$((`echo $PIDS | wc -w`-1)) # count the pids and substract ourselves
	#allow for max 1 process to be up in the same time
	[ $stress_no -ge 1 ] && \
		log2flash "WARN utils.sh: don't run, system under stress ($stress_no)" &&\
		return 0

	touch ${UTILS_FILE}
	load_env             > /dev/null
	check_free_memory    > /dev/null
	check_login          > /dev/null
	check_flash          > /dev/null
	check_temp_humid     > /dev/null
	check_battery        > /dev/null
	check_config_changed > /dev/null
	check_time			 > /dev/null
	check_udhcpc		 > /dev/null
	return 0
}

check_all
#comm/sms_monitor.sh &
