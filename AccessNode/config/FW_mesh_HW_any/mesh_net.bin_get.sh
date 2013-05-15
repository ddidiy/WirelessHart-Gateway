#!/bin/sh


FILE_UPDATE_READY=/tmp/update_mesh_ended.txt 
FILE_UPDATE_SIG=/tmp/update_mesh.sig

if [ `ps | grep "node_connection" | grep mesh | wc -l` -lt 1 ]; then
    echo "Mesh process is not started yet. Unable to SYNC mesh list."
    return 0
fi

rm -f ${FILE_UPDATE_READY}
touch ${FILE_UPDATE_SIG}
killall -USR2 node_connection

sleep 1
local counter=0

while [ ! -f ${FILE_UPDATE_READY} ]; do 
	sleep 3
	counter=$(($((counter))+1))
	# don't wait more than 60 seconds (3*20)
	[ $counter -ge 20 ] && echo "Warning, cannot SYNC mesh list. Please try again later" && return 0
done

rm -f ${FILE_UPDATE_READY}
