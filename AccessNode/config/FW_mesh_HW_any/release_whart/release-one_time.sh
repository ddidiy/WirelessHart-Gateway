#!/bin/sh

user_name=nisa100

echo "Add user $user_name"
tmp=`cat /etc/passwd | grep "^${user_name}:" | wc -l`
if [ $tmp -eq 0 ]; then
	echo "${user_name}:\$1\$\$PM5UEKvvaaDRhCM3OhM1E0:0:0:${user_name}:/access_node/firmware/:cfg.sh" >> /etc/passwd 
fi


