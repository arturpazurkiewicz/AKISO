#!/bin/bash
units(){
	echo
	if [[ $1 -lt 1000 ]]; then
		echo "$1 B/s"
	elif [[ $1 -lt 1000000 ]]; then
		let output=$1/1000
		echo "$output kB/s"
	else
		let output=$1/1000000
		echo "$output MB/s"
	fi
}
function show_time () {
    num=$1
    min=0
    hour=0
    day=0
    if((num>59));then
        ((sec=num%60))
        ((num=num/60))
        if((num>59));then
            ((min=num%60))
            ((num=num/60))
            if((num>23));then
                ((hour=num%24))
                ((day=num/24))
            else
                ((hour=num))
            fi
        else
            ((min=num))
        fi
    else
        ((sec=num))
    fi
    echo "$day"d "$hour"h "$min"m "$sec"s
}
sumRecBytes=0
recBytes=0
sumTrBytes=0
trBytes=0
counter=1
starttx=$(awk 'NR>2{print $10}' /proc/net/dev | paste -sd+ | bc )
startrx=$(awk 'NR>2{print $2}' /proc/net/dev | paste -sd+ | bc )
lastrecBytes=$startrx
lasttrBytes=$starttx
rm 'graph.dat'
while true
do
	clear
	load=$(cat /proc/loadavg | awk '{print $1}')
	time=$(cat /proc/uptime | awk '{print $1}')
	battery=$(cat /sys/class/power_supply/BAT0/uevent | grep 'CAPACITY=' | cut -f2 -d"=")
	let "recBytes = $(awk 'NR>2{print $2}' /proc/net/dev | paste -sd+ | bc ) - lastrecBytes"
	lastrecBytes=$(awk 'NR>2{print $2}' /proc/net/dev | paste -sd+ | bc )
	let "sumRecBytes = ($(awk 'NR>2{print $2}' /proc/net/dev | paste -sd+ | bc ) - startrx)/counter"
	let "trBytes = $(awk 'NR>2{print $10}' /proc/net/dev | paste -sd+ | bc ) - lasttrBytes"
	lasttrBytes=$(awk 'NR>2{print $10}' /proc/net/dev | paste -sd+ | bc )
	let "sumTrBytes = ($(awk 'NR>2{print $10}' /proc/net/dev | paste -sd+ | bc ) - starttx)/counter"
	echo Upload: $(units $trBytes) Average Upload: $(units $sumTrBytes)
	echo Download: $(units $recBytes) Average Download: $(units $sumRecBytes)
	echo Battery: $battery%
	echo Computer working:  $(show_time ${time%.*})
	echo Task load: $load
	let counter=$counter+1
	echo $counter $recBytes >> graph.dat
	gnuplot -e "set terminal dumb;
	set xrange [$counter-25:$counter];
	plot 'graph.dat' using 2: xtic(1) with boxes title '';
	"
	sleep 1
done

