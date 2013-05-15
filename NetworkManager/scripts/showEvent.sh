
#!/bin/bash

if [ $# -ne 2 ] 
then
	echo "Invalid parameters, expecting event id and log folder; ex :" $0 "121 /tmp";
	exit;
fi;

if echo $1 | grep "^[0-9]*$">/dev/null
then  
	echo "Search for event with id="$1"; display when it was generated and eventually confirmed";
else
	echo "Event id has to be a number!"
	exit;
fi

if [ -d $2 ]
then  
	echo "Searching for event with id="$1"; in folder $2";
else
	echo "Invalid parameters, expecting event id and log folder; ex :" $0 "121 /tmp";
	exit;
fi

FOLDER_LOG=$2;

eventId="evId="$1", ";
#echo eventId=$eventId;
if [ `grep Queue $FOLDER_LOG/nm.lo* | grep -c $eventId` -eq 0 ]
then 
	echo "There is no event with id="$1;
	exit;
fi;


echo "+++++++++++++++"

grep Queue $FOLDER_LOG/nm.log | grep $eventId;
opEvIds=`grep Queue $FOLDER_LOG/nm.log | grep $eventId | grep  logNewEvent | awk 'BEGIN { FS = "Ids: " }; {print $2};' | sed 's/\]//g' | cut -d\; -f1`;
echo opEvIds: $opEvIds

echo "+++++++++++++++"

operationIds=`echo $opEvIds | sed 's/ /\n/g' | awk 'BEGIN { FS = "[" }; {print " -e \"id= *"  $1  ",\""; };'`
#echo "operationIds :" $operationIds
eval "grep " $operationIds "$FOLDER_LOG/nmOperations.lo*"
 
echo "+++++++++++++++"

operationIds=`echo $opEvIds | sed 's/ /\n/g' | awk 'BEGIN { FS = "[" }; {print " -e \"id= *"  $1  ",\""; };'`

#echo $eventId 
eval "grep " \"$eventId\" "$FOLDER_LOG/nm.lo*"

echo "+++++++++++++++"
