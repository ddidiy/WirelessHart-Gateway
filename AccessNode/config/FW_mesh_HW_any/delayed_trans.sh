#!/bin/sh

DELAY_TRANS=${1:-0}

module_monitor s 13 1 > /dev/null		#Fake module up
{
	sleep $DELAY_TRANS
	module_monitor s 11 1 > /dev/null		#history module up (to avoid race condition)
	killall -USR2 history
	module_monitor s 13 0 > /dev/null		#Fake module down
}&