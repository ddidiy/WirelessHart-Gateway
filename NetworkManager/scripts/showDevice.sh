#!/bin/bash

if [ $# -ne 2 ] 
then
	echo "Invalid paramters, expecting device address on 32 bits in hexa and a log folder; ex :" $0 "334A /tmp";
	exit;
fi;

if [ -d $2 ]
then  
	echo "Searching for device with="$1"; in folder $2";
else
	echo "Invalid parameters, expecting event id and log folder; ex :" $0 "121 /tmp";
	exit;
fi


#echo $1 | awk '/[0-9a-zA-Z]{4}/; { print $1}';
if [ `echo "${#1}"` -ne 4  ]
then
	echo "Device address has to be like : AAAA";
	exit;
fi

FOLDER_LOG=$2;
echo "Device : " $1

deviceJoins=`grep "InitJoin" $FOLDER_LOG/nm.lo* | grep $1 | wc -l`
echo "\$deviceJoins: " $deviceJoins "for Device: " $1;

grep InitJ $FOLDER_LOG/nm.lo* | grep  ":"$1
