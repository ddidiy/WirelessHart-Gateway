#!/bin/sh
#this script is used to generate AN alarms

#print usage to screen
print_usage()
{
	echo ""
	echo $1
	echo ""
	echo "Usage: ${0} <evt_id> <evt_details> [<extra_details>]"
	echo "Where "
	echo "    <evt_id> is the event ID to generate"
	echo "    <evt_details> is the event details"
	echo ""
}

[ ! "$1" ] && log2flash "ERR ${0}: must specify event code" && print_usage "No event code provided" &&  return 1
[ ! "$2" ] && log2flash "ERR ${0}: must specify event details" && print_usage "No event details provided" &&  return 2

EVENT_CODE=$1
EVENT_DETAILS=$2
EXTRA_DETAILS=$3

echo "NOT YET IMPLEMENTED"