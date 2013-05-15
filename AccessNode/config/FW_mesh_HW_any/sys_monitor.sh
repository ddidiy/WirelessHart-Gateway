#!/bin/sh

CMD1="uptime"
#TAKE CARE: CMD2 is used by modules_dead; change there when changing CMD2
CMD2="ps | grep -e node_connection -e cc_comm -e history -e remote_access -e scheduler -e user_interface | grep -v grep"
CMD3="free | grep Mem"
CMD4="du -hd0 /tmp /access_node/"
CMD5="echo pidfiles=\`ls -l /tmp/*.pid | wc -l\`"
CMD6="echo ntp_drift=\`cat /etc/ntp/drift\`"
CMDS="$CMD1,$CMD2,$CMD3,$CMD4,$CMD5,$CMD6"


SYS_LOG_TMP=${NIVIS_TMP}/syslog.txt
SYS_LOG_SAVE=${NIVIS_ACTIVITY_FILES}/syslog.txt

#limit is in bytes
TMP_LOG_SIZE_LIMIT=2000
SAVE_LOG_SIZE_LIMIT=256000

LOG_LAST=0
LOG_INTERVAL=600

CLEAR_DEAD_LAST=0
CLEAR_DEAD_INTERVAL=2700

DEAD_FLAG=0


get_file_size()
{
	ls -Ll $1 | tr -s ' ' | cut -f 5 -d ' '
}

modules_dead()
{
	if [ `eval $CMD2 | wc -l` -ge 7 ]; then
		return 1
	fi
	return 0
}

save_log_to_flash()
{
	cat $SYS_LOG_TMP >> $SYS_LOG_SAVE
	echo "" > $SYS_LOG_TMP
}

manage_log_size()
{
	local tmp_size=`get_file_size $SYS_LOG_TMP`

	if [ $tmp_size -ge $TMP_LOG_SIZE_LIMIT ]; then
		save_log_to_flash
	fi

	local save_size=`get_file_size $SYS_LOG_SAVE`

	if [ $save_size -ge $SAVE_LOG_SIZE_LIMIT ]; then
		mv $SYS_LOG_SAVE ${SYS_LOG_SAVE}.1
		touch $SYS_LOG_SAVE
	fi
}


while [ true ]; do
	if [ $((`date +%s`- $LOG_LAST)) -ge $LOG_INTERVAL ] ; then
		LOG_LAST=`date +%s`
		touch $SYS_LOG_TMP
		(IFS=','
		echo ====
		date
		for i in $CMDS; do
			eval $i
		done) >> $SYS_LOG_TMP
		manage_log_size
	fi

	if [ $((`date +%s`- $CLEAR_DEAD_LAST)) -ge $CLEAR_DEAD_INTERVAL ] ; then
		CLEAR_DEAD_LAST=`date +%s`
		DEAD_FLAG=0
	fi

	modules_dead
	if [ $? -eq 0 -a $DEAD_FLAG -eq 0 ]; then
		DEAD_FLAG=1
		save_log_to_flash
		take_system_snapshot.sh ${NIVIS_ACTIVITY_FILES}/snapshot_crash.txt ;
	fi
	sleep 60
done
