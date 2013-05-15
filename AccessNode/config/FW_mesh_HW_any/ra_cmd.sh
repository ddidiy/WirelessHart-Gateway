#!/bin/sh
#this script is used to execute RA commands from scheduler

#print usage to screen
print_usage()
{
	echo ""
	echo $1
	echo ""
	echo "Usage: $0 <evt_id> <cmd_code> <parameters>"
	echo "Where "
	echo "    <evt_id> is the resulting event ID"
	echo "    <cmd_code> is the command code to be executed"
	echo "    <parameters> ar command parameters "
	echo ""
	return 0
}

[ ! "$1" ] && log2flash "ERR ${0}: must specify event code" && print_usage "No event code provided" &&  return 1
[ ! "$2" ] && log2flash "ERR ${0}: must specify command code" && print_usage "No command code provided" &&  return 2
[ ! "$3" ] && log2flash "ERR ${0}: must specify command parameters" && print_usage "No command parameters provided" &&  return 3

EVENT_CODE=$1
COMMAND_CODE=$2
shift 2
COMMAND_PARAMETERS=$*

if [ $COMMAND_CODE -eq 5 ]; then 
  echo "start.sh"
  start.sh
else
  echo "raise_an_ev -e $EVENT_CODE -c $COMMAND_CODE -v \"$COMMAND_PARAMETERS\""
  raise_an_ev -e $EVENT_CODE -c $COMMAND_CODE -v "$COMMAND_PARAMETERS"
fi  

