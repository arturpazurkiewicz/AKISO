#!/bin/bash
i=1;
echo "test"
while [[ i -le 64 ]]
do
	if [[ !( i -eq 9 || i -eq 19 || i -eq 32 || i -eq 33 ) ]]; then
		kill -$i $1
		#sleep 0.0001
	fi
	
	i=$((i+1))
	
done