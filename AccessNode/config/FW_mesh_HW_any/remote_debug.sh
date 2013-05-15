#!/bin/sh

# share common function 
. common.sh

LOG=${NIVIS_TMP}remote_debug.log

FOLDER_USB_STORAGE=/mnt/usbstorage/

DEBUG_WITH_NC=debug_with_nc
DEBUG_WITH_FTP=debug_with_ftp
DEBUG_WITH_SCRIPT=debug_with_script.sh

NC_IP=10.32.0.12
NC_PORT=27048

FTP_IP=10.32.0.12
FTP_USER=nivis
FTP_PASS=nivis
FTPDIR_BASE=/tmp/cla/

FTPDIR=$FTPDIR_BASE`hostname`

FTP_CMD_RC=/access_node/dbg_rc
FTP_CMD="ftp_cmd.sh"
FTP_OUTPUT=${NIVIS_TMP}ftp_output.txt

FTP_DEBUG_ON=${NIVIS_TMP}debug_ftp_on
NC_DEBUG_ON=${NIVIS_TMP}debug_nc_on
SH_DEBUG_ON=${NIVIS_TMP}debug_sh_on

log()
{
	echo -n "`date \"+%Y-%m-%m %T\"` " >> $LOG
	echo  "$*" >> $LOG
}

# $1 - debug type:  debug_with_nc, debug_with_ftp or debug_with_script.sh
# output folder of request
detect_request()
{
	[ -f ${FOLDER_USB_STORAGE}$1 ] && echo ${FOLDER_USB_STORAGE} && return 0
	[ -f ${NIVIS_TMP}$1 ] && echo ${NIVIS_TMP} && return 0	
	[ -f ${NIVIS_ACTIVITY_FILES}$1 ] && echo ${NIVIS_ACTIVITY_FILES} && return 0
	echo ""
}

# $1-$5 ftp requests 
ftp_compose_req()
{
	local cmd
	echo "machine $FTP_IP login $FTP_USER password $FTP_PASS" > $FTP_CMD_RC
	echo "macdef init" >> $FTP_CMD_RC
	echo "prompt"  >> $FTP_CMD_RC
	echo "bin"  >> $FTP_CMD_RC	
	
	[ ! -z "$1" ] && echo "$1"  >> $FTP_CMD_RC			
	[ ! -z "$2" ] && echo "$2"  >> $FTP_CMD_RC				
	[ ! -z "$3" ] && echo "$3"  >> $FTP_CMD_RC				
	[ ! -z "$4" ] && echo "$4"  >> $FTP_CMD_RC				
	[ ! -z "$5" ] && echo "$5"  >> $FTP_CMD_RC					
	
	echo "by"  >> $FTP_CMD_RC				
	echo ""  >> $FTP_CMD_RC					
	echo ""  >> $FTP_CMD_RC					
}

ftp_set_rc_req()
{
	(cd /access_node/; rm -f .netrc; ln -s $FTP_CMD_RC .netrc; )		
}

ftp_set_rc_def()
{
	(cd /access_node/; rm -f .netrc; ln -s ./etc/netrc .netrc; )			
}

# $1 - output file
ftp_run_req()
{
	ftp_set_rc_req	
	
	if [ ! -z "$1" ]; then
		ftp $FTP_IP > $1
	else
		ftp $FTP_IP >> $LOG
	fi		 
	
	if [ $? -ne 0 ]; then
	    log "failed to connect to $FTP_IP"
		ftp_set_rc_def
		return 1
	fi
	
	ftp_set_rc_def
	return 0
}


ftp_debug()
{
	# get ftp_cmd.sh
	# mv  ftp_cmd.sh.time.got
	# execute ftp_cmd.sh > ftp_cmd.sh.time.result
	# put ftp_cmd.sh.time.result
	
	[ -f $FTP_CMD ] && rm -f $FTP_CMD 
	
	log "get $FTP_CMD"
	ftp_compose_req "cd $FTPDIR" "get $FTP_CMD"
	ftp_run_req $FTP_OUTPUT || return 1 
		
	if [ ! -f $FTP_CMD ]; then
		grep "250 CWD command successful" $FTP_OUTPUT >> $LOG
		if [ $? -eq 0 ]; then
			log "folder $FTPDIR exist but no $FTP_CMD"	
			return 1
		fi
		log "Try to make folder "
		ftp_compose_req "cd $FTPDIR_BASE" "mkdir `hostname`"
		ftp_run_req 
				
		return 1
	fi  
	
	exe_time=`date +%s`
	
	log "rename $FTP_CMD $FTP_CMD.${exe_time}.dl"
	ftp_compose_req "cd $FTPDIR" "rename $FTP_CMD $FTP_CMD.${exe_time}.dl"  
	ftp_run_req
	
	chmod a+x $FTP_CMD
	
	log "Executing... $FTP_CMD"
	
	$FTP_CMD > $FTP_CMD.${exe_time}.result

	log "ftp put $FTP_CMD.${exe_time}.result"
	ftp_compose_req "cd $FTPDIR" "put $FTP_CMD.${exe_time}.result"  
	ftp_run_req

	mv $FTP_CMD.${exe_time}.result $FTP_CMD.result.last	
	mv $FTP_CMD $FTP_CMD.last	
	
	#or rm -f  $FTP_CMD.${exe_time}.result
}


touch $LOG 

SLEEP_INT_DEF=900
SLEEP_INT=900



while [ true ]; do
    #detect 
	req_folder=`detect_request $DEBUG_WITH_SCRIPT`

	if [ ! -z $req_folder ] ; then
		if [ ! -f $SH_DEBUG_ON ]; then
			touch $SH_DEBUG_ON 
			log2flash "Remote debug requested: ${req_folder}${DEBUG_WITH_SCRIPT}"
		fi					
		
		exe_time=`date +%s`
		log "mv ${req_folder}${DEBUG_WITH_SCRIPT} ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending"
		mv ${req_folder}${DEBUG_WITH_SCRIPT} ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending
		chmod a+x ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending
		log "Executing...${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending" 
		${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending > ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.result
		log "mv ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.done"		
		mv ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.pending ${req_folder}${DEBUG_WITH_SCRIPT}.${exe_time}.done						
	else
		[ -f $SH_DEBUG_ON ] && rm -f $SH_DEBUG_ON  			
	fi	

	req_folder=`detect_request $DEBUG_WITH_FTP`

	if [ ! -z $req_folder ] ; then
		if [ ! -f $FTP_DEBUG_ON ]; then 
			touch $FTP_DEBUG_ON 
			log2flash "Remote debug requested: ${req_folder}${DEBUG_WITH_FTP}"		
		fi
        ftp_debug
	else
		[ -f $FTP_DEBUG_ON ] && rm -f $FTP_DEBUG_ON  	
	fi
	
	req_folder=`detect_request $DEBUG_WITH_NC`

	if [ ! -z $req_folder ] ; then
		if [ ! -f $NC_DEBUG_ON ]; then
			touch $NC_DEBUG_ON 
			log2flash "Remote debug requested: ${req_folder}${DEBUG_WITH_NC}"
			log2flash "nc -e /bin/sh $NC_IP $NC_PORT"
		fi	
		log "nc -e /bin/sh $NC_IP $NC_PORT"
		nc -e /bin/sh $NC_IP $NC_PORT		
	else
		[ -f $NC_DEBUG_ON ] && rm -f $NC_DEBUG_ON  	
	fi	
			
	sleep $SLEEP_INT
	SLEEP_INT=$SLEEP_INT_DEF 
done 
