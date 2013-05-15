#!/bin/sh


FILE_UPDATE_SIG=/tmp/update_mesh_tmp.sig
FILE_NET_BIN=/tmp/mesh_net_tmp.bin

rm -f ${FILE_NET_BIN}
touch ${FILE_UPDATE_SIG}
killall -USR2 node_connection

sleep 1

while [ ! -f ${FILE_NET_BIN} ]; do 
	sleep 3
done




