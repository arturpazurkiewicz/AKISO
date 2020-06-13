#!/bin/bash
echo "PID	FILES	  State			Name"
for pid in $(ls /proc | grep '[0-9]')
	do
	file=/proc/$pid/status
	if [ -f "$file" ]; then
		fileCount=$(sudo ls /proc/$pid/fd | wc -w)
		state=$(cat $file | grep 'State:'| awk '{print $3}')
		name=$(cat $file | grep 'Name:'| awk '{print $2}')
		echo "$pid	$fileCount	  $state		$name"
	fi
done