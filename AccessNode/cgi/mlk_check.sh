#! /bin/bash

declare -a requests
#Open file for reading to array
exec 10<>./calls

while read LINE <&10; do
    requests[$count]=$LINE
    ((count++))
done
exec 10>&-

element_count=${#requests[@]}
index=0

while [ "$index" -lt "$element_count" ] ; do
	REQ=${requests[$index]}
	export CONTENT_LENGTH=`echo $REQ |wc -c`
	echo --------------------
	echo "Sending: $REQ"
	echo --------------------
	echo $REQ  | curl -v  -H "Content-Type: text/plain" -c cookies.txt -b cookies.txt -d @- http://10.32.0.122:8083/rpc.cgi
	#read temp_var
	echo ====================
	echo
	let "index = $index + 1"
done
