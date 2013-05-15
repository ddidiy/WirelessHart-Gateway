#!/bin/bash

# web root folder name
WEB_ROOT="/var/www"

# python script name: this one does everything
SCRIPT=RRDWebLogger.py

# check for write access to web root
[ -w $WEB_ROOT ] && W="Write = yes" || W="Write = no"

if [ "$W" == "Write = no" ]; 
then 
	echo "You do not have write access to www root folder:";
	echo $WEB_ROOT;
	exit
fi

# checks if script is already running
if ps ax | grep -v grep | grep $SCRIPT | grep -v edit | grep -v emacs | grep -v joe | grep -v vi > /dev/null
then
    echo "$SCRIPT is already running .."
	exit
fi

# run the python script
chmod a+x $SCRIPT
./RRDWebLogger.py $WEB_ROOT &

