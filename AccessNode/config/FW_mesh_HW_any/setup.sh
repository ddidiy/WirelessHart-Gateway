#!/bin/sh

ANID_VAL="000000"
APPID_VAL="0001"
PROXY_VAL="65.205.163.69:11500"
MESH_IF_VAL="010"
SN_OWNER_ID_VAL="0000"
AN_OWNER_ID_VAL="0000"
CONFIRM="n"
ETH_ENABLED=0
ONLY_PROVIDED=0
QUERY_MODE=0

CONFIG_INI_FILE="${NIVIS_PROFILE}config.ini"
CONFIG_INI_TEMP="/tmp/config.ini.temp"

RULE_FILE="${NIVIS_PROFILE}rule_file.cfg"
RULE_TEMP="/tmp/rule_file.cfg.temp"

START_FILE="${NIVIS_FIRMWARE}start.sh"
START_TEMP="/tmp/start.sh.temp"

PPPD_DESTIONATIONS_FILE="${NIVIS_FIRMWARE}../PPPD_DESTINATIONS.cfg"

. common.sh

parameter()
{
	echo $1 | sed "s/.*=//"
}

print_usage_and_exit()
{
	echo ""
	echo $1
	echo ""
	echo "Usage: $0 OPTION"
	echo "Where OPTION is:"
	echo "    [-a|--anid=  <an_id>]"
	echo "    [-d|--appid= <app_id>]"
	echo "    [-p|--proxy= <proxy_ip:proxy_port>]"
	echo "    [-m|--mesh=  <mesh_interface>]"
	echo "    [-i|--sn_ownerid= <sn_owner_id_list>]"
	echo "    [-n|--an_ownerid= <an_owner_id>]"
	echo "    [-o|--only]"
	echo "    [-e|--eth]"  
	echo "    [-q|--query]"
	echo "    [-y]"
	echo "    [-h|--help]"
	echo "Parameter meaning:"
	echo "  <an_id>  is AN  ID, 3 hex digits, of form AABBCC"
	echo "  <app_id> is APP ID, 2 hex digits, of form AABB"
	echo "  <proxy_ip:proxy_port> is Data Center ip:port"
	echo "  <mesh_interface> is Mesh interface, 3 binary digits form BBB"
	echo "  <sn_owner_id_list> is SN Owner ID list, comma separated, each owner 4 hex chars"
	echo "  <an_owner_id> is AN Owner ID, 4 hex chars"
	echo "  -o|--only change only parameters received in cmd line. NOT IMPLEMENTED YET."
	echo "  -e|--eth use ETH as DC comm method. If 0, use GPRS on slot 1 as DC comm method"
	echo "  -q|--query ask values for all parameters"
	echo "  -y force YES answer to confirmation question"
	echo "  -h|--help print this screen"
	echo ""
	exit
}

assign_parameters()
{
	ret_value=1

	case $1 in
	-a)			ANID_VAL=$2; 	ret_value=2		;;
	-d)			APPID_VAL=$2;	ret_value=2		;;
	-p)			PROXY_VAL=$2;	ret_value=2	    ;;
	-m)			MESH_IF_VAL=$2;	ret_value=2	    ;;
	-i)			SN_OWNER_ID_VAL=$2;	ret_value=2	;;
	-n)			AN_OWNER_ID_VAL=$2;	ret_value=2	;;
	-y)			CONFIRM="y"			            ;;
	-o | --only | -only)	ONLY_PROVIDED=1 ;;
	-e | --eth  | -eth)		ETH_ENABLED=1   ;;
	-q | --query | -query)	QUERY_MODE=1	;;
	--anid=* | -anid=*)		ANID_VAL=`parameter	$1`		;;
	--appid=* | -appid=*)	APPID_VAL=`parameter $1`    ;;
	--proxy=* | -proxy=*)	PROXY_VAL=`parameter $1`	;;
	--mesh=* | -mesh=*)		MESH_IF_VAL=`parameter $1`	;;
	--sn_ownerid=* | -sn_ownerid=*)	SN_OWNER_ID_VAL=`parameter $1`	;;
	--an_ownerid=* | -an_ownerid=*)	AN_OWNER_ID_VAL=`parameter $1`	;;
	-h | --help | -help)	print_usage_and_exit "Help screen:" ;;
	*)	print_usage_and_exit "Unsupported option \"$1\"" ;;
	esac
	return $ret_value
}

dispatch_parameters( )
{
	while [ $# != 0 ] ; do
		assign_parameters $*
		shift $?
	done
}

check_parameters()
{
	#echo without -n add an extra char: newline, so we compare with 7 instead of 6
	#this behavior is consistent on x86/ARM 
	[ `echo $ANID_VAL | sed s/[\^0-9,a-f,A-F]//g |  wc -c` -ne 7 -o\
	  `echo $ANID_VAL | wc -c` -ne 7\
	]&& print_usage_and_exit "AN ID $ANID_VAL is invalid"

	[ `echo $APPID_VAL | sed s/[\^0-9,a-f,A-F]//g |  wc -c` -ne 5 -o\
	  `echo $APPID_VAL | wc -c` -ne 5\
	]&& print_usage_and_exit "APP ID $APPID_VAL is invalid"
	
	[ `echo $PROXY_VAL |\
		sed -n "/[0-9]\+\.[0-9]\+\.[0-9]\+\.[0-9]\+\:[0-9]\+/p" |\
		wc -l` -eq 0\
	] && print_usage_and_exit "Value [$PROXY_VAL] is invalid as ip:port pair"
	
	#same reason as before to compare with 4 instead of 3 
	[ `echo $MESH_IF_VAL | sed s/[\^0-1]//g | wc -c` -ne 4 -o\
	  `echo $MESH_IF_VAL | wc -c` -ne 4 \
	] && print_usage_and_exit "MESH INTERFACE must be 3 binary digits. $MESH_IF_VAL is invalid"
	
	#TODO:  check for validity of other parameters: OWNER_ID_VAL
}

request_parameters()
{
	echo -n "  AN ID [$ANID_VAL]: "
	read tmp
	ANID_VAL=${ANID_VAL:-$tmp}

	echo -n "  APP ID [$APPID_VAL]: "
	read tmp
	APPID_VAL=${APPID_VAL:-$tmp}

	echo -n "  PROXY [$PROXY_VAL]: "
	read tmp
	PROXY_VAL=${PROXY_VAL:-$tmp}

	echo -n "  MESH INTERFACE [$MESH_IF_VAL]: "
	read tmp
	MESH_IF_VAL=${MESH_IF_VAL:-$tmp}

	echo -n "  SN OWNER ID LIST [$SN_OWNER_ID_VAL]: "
	read tmp
	OWNER_ID_VAL=${SN_OWNER_ID_VAL:-$tmp}
	
	
	echo -n "  AN OWNER ID [$SN_OWNER_ID_VAL]: "
	read tmp
	OWNER_ID_VAL=${SN_OWNER_ID_VAL:-$tmp}
	
	echo -n "  ETH enabled [`if [ $ETH_ENABLED -eq 1 ] ; then echo 'YES'; else echo 'NO'; fi`]: "
	read tmp
	if [ $tmp ]; then
		case $tmp in
			y|yes|Y|YES)	ETH_ENABLED=1 ;;
			*)				ETH_ENABLED=0 ;;
		esac
	fi
}

do_work()
{
	echo "Configuring node with:"
	echo "  AN ID:            $ANID_VAL"
	echo "  APP ID:           $APPID_VAL"
	echo "  PROXY:            $PROXY_VAL"
	echo "  MESH INTERFACE:   $MESH_IF_VAL"
	echo "  SN OWNER ID LIST: $SN_OWNER_ID_VAL"
	echo "  THIS AN OWNER ID: $AN_OWNER_ID_VAL"
	echo "  ETH ENABLED:      `if [ $ETH_ENABLED  -eq 1 ] ; then echo 'YES'; else echo 'NO'; fi`"

	if [ $CONFIRM != "y" ]; then
		echo -n "Confirm changes? "
		read CONFIRM
	fi

	case $CONFIRM in
	y | Y | yes | YES | Yes | yES)
		echo "Apply changes"
		
		ANID_OLD=`grep "AN_ID" $CONFIG_INI_FILE| cut -d'#' -f 1 |
					sed "s/.*AN_ID *= *00 \(......\).*/\1/I"`
		ANID_OLD=${ANID_OLD:-"000000"}

		echo "Updating in $CONFIG_INI_FILE: AN_ID, APP_ID, PROXY"
		echo "    ETHER_LINK, MESH_INTERFACE"
		echo "    MESH_OWNER_ID_LIST, THIS_AN_OWNER_ID"
		sed "	s/\(PROXY.*=[^0-9]*\)\([0-9]\|\.\|\:\)*/\1$PROXY_VAL/Ig
			 	s/\(ETHER_LINK.*=.*\)[0-9]*/\1$ETH_ENABLED/Ig
			 	s/\(MESH_INTERFACE.*=.*\)[0-1][0-1][0-1]/\1$MESH_IF_VAL/Ig
			 	s/\(MESH_OWNER_ID_LIST\).*/\1 = $SN_OWNER_ID_VAL/Ig
			 	s/\(MESH_AN_OWNER_ID\).*/\1 = $AN_OWNER_ID_VAL/Ig
				s/\(APP_ID\).*/\1 = $APPID_VAL/Ig
				s/\(AN_ID\).*/\1 = 00 $ANID_VAL/Ig"\
			$CONFIG_INI_FILE > $CONFIG_INI_TEMP && \
			mv $CONFIG_INI_TEMP $CONFIG_INI_FILE

		echo "Updating AN_ID in rule file"
		sed "s/AN__ID\|$ANID_OLD/$ANID_VAL/Ig" $RULE_FILE > $RULE_TEMP &&\
		mv $RULE_TEMP $RULE_FILE

# detect_cc_comm.sh will override this...
# if gprs is deteced, it will be used
# use MODEM_PWR_LINE_INDEX=-1 or ETHER_LINK=1 to disable gprs...
#		if [ $ETH_ENABLED ]; then
#			echo "Disable GPRS script -> USE ETH link"
#			ln -sf comm/default_ctl.sh set_cc_modem.sh
#		else
#			echo "Enable GPRS script for -> USE GPRS link"
#			ln -sf comm/gprs_ctl.sh set_cc_modem.sh
#		fi

		PROXY_IP=`echo $PROXY_VAL | cut -d':' -f 1`
		echo "set route to  $PROXY_IP into  $PPPD_DESTIONATIONS_FILE"  
		echo $PROXY_IP > $PPPD_DESTIONATIONS_FILE

	;;
	*)
		echo "Configuration files NOT changed."
	;;
	esac
}

dispatch_parameters $*

if [ $QUERY_MODE -eq 1 -o $# -eq 0 ]; then
	request_parameters
fi

[ -z ${NIVIS_PROFILE}  ] && echo "WARNING: variable NIVIS_PROFILE not set"
[ -z ${NIVIS_FIRMWARE} ] && echo "WARNING: variable NIVIS_FIRMWARE not set"

check_parameters
do_work


