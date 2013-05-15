touch /tmp/mesh_report.sig
killall -USR2 node_connection
sleep 1

while [ true ]; do 
	tail -n 2 /tmp/MeshReport.txt | grep "#end_file" > /dev/null &&  exit 0 
	sleep 3
done



