#!/bin/bash
echo
echo Send a SIGUSR2 signal to the current instance of the NetworkManager to read the commands from the sm_command.ini.
echo If there would be any topology write request then the *.dot files will be generated to the subnetCaptures folder.
echo 

TMPDATE=`date +%Y.%m.%d_%H.%M.%S.%N`
SUBNET_CAPTURES_FOLDER=subnetCaptures

if [ -d $SUBNET_CAPTURES_FOLDER ] ;  then
	echo;
else
	echo create folder $SUBNET_CAPTURES_FOLDER
	mkdir $SUBNET_CAPTURES_FOLDER 
fi

if [ `ls -1 subnetCaptures/ | grep -c '\.dot'` -gt 0 ] ; then
	echo back up the existing files 
	echo create folder subnetCaptures/$TMPDATE
	mkdir $SUBNET_CAPTURES_FOLDER/$TMPDATE
			
	echo move existing files to subnetCaptures/$TMPDATE
	folder=$SUBNET_CAPTURES_FOLDER/$TMPDATE
	
	mv $SUBNET_CAPTURES_FOLDER/*\.dot* $folder
fi 

# send the signal
echo sends signal SIGUSR2 to NetworkManager ...
kill -s SIGUSR2 `pgrep WHart_NM.o`

# try to transform the dot files to image files if there would be any dot file generated
#sleep 1
#echo transforms the *dot files to *png
#for name in `find $SUBNET_CAPTURES_FOLDER -maxdepth 1 -iname '*.dot'`; do   
#	echo '    ' $name ---  $name.png; 
#	dot $name -Tpng > $name.png;
#done;

#echo 
