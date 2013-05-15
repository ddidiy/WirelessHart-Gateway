#!/bin/bash

if [ $# -ne 1 ] 
then
	echo "Invalid parameters, expecting logs folder; ex :" $0 " /tmp";
	exit;
fi;

if [ -d $1 ]
then  
	echo "Searching in folder $2";
else
	echo "Invalid parameters, expecting logs folder; ex :" $0 " /tmp";
	exit;
fi


FOLDER_LOG=$1;

echo ++++++++++++++++++++++++++++

grep -e NodeVisibleVerifier -e AlarmDispatcher -e VisibleEdgeFlow $FOLDER_LOG/nm.lo* 

echo ++++++++++++++++++++++++++++

echo ++++++++++++++++++++++++++++
