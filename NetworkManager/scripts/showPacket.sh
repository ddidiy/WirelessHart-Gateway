#!/bin/bash

if [ $# -ne 2 ] 
then
	echo "Invalid parameters, expecting packet handle and a log folder; ex :" $0 "287 /tmp";
	exit;
fi;

if echo $1 | grep "^[0-9]*$">/dev/null
then  
	echo;
else
	echo "Packet handle has to be a number!"
	exit;
fi

if [ -d $2 ]
then  
	echo "Searching for packet with handle="$1"; in folder $2";
else
	echo "Packet handle has to be a number"
	exit;
fi

# 02-23 09:01:53,405  INFO h7.n.o.WHOperationQueue 201: logSendPacket() : > [Proxy=0xf981, UniqueID=0xf870003301], handle=12, op[Ev]Ids: 33[5] 35[5]
# 02-23 09:01:53,462  INFO h7.n.o.WHOperationQueue 234: logConfPacket() : < [Proxy=0xf981, UniqueID=0xf870003301], handle=12, opIds: 33 35 ;  DT=0

FOLDER_LOG=$2;
packetHandle="handle="$1", ";
echo packetHandle=$packetHandle;
if [ `grep Queue $FOLDER_LOG/nm.lo* | grep -c $packetHandle` -eq 0 ]
then 
	echo "There is no packet with handle="$1;
	exit;
fi;

echo "+++++++++++++++"

grep Queue $FOLDER_LOG/nm.log | grep $packetHandle;
opEvIds=`grep Queue $FOLDER_LOG/nm.log | grep $packetHandle | grep  logSendPacket | awk 'BEGIN { FS = "Ids: " }; {print $2};' | sed 's/\]//g'`;
echo opEvIds: $opEvIds

echo "+++++++++++++++"

operationIds=`echo $opEvIds | sed 's/ /\n/g' | awk 'BEGIN { FS = "[" }; {print " -e \"id= *"  $1  ",\""; };'`
#echo "operationIds :" $operationIds
eval "grep " $operationIds "$FOLDER_LOG/nmOperations.lo*"
 
echo "+++++++++++++++"

operationIds=`echo $opEvIds | sed 's/ /\n/g' | awk 'BEGIN { FS = "[" }; {print " -e \"id= *"  $1  ",\""; };'`
    eventIds=`echo $opEvIds | sed 's/ /\n/g' | awk 'BEGIN { FS = "[" }; {print " -e \"logNewEvent() : evId=" $2 ",\"";  };' | sort -u`
#echo "eventIds :" $eventIds
eval "grep " $eventIds "$FOLDER_LOG/nm.lo*"

echo "+++++++++++++++"
