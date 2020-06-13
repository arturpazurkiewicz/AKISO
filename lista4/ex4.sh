#!/bin/bash
git init >> /tmp/temp
touch strona.txt
touch old.txt
git add strona.txt
while (true) do
	lynx -dump $1 > new.txt
	if [ -z "$(diff -q -b old.txt new.txt)" ]; then
		echo nic
	else
		cat new.txt > "strona.txt"
		cat new.txt > old.txt
		git commit -a -m "$1" >>/tmp/temp
		echo znalezione
		zenity --notification\
    	--window-icon="info" \
    	--text="Strona $1 została zmodyfikowana!"
	fi
	sleep $2
done