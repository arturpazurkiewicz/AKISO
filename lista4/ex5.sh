#!/bin/bash
reset
trap "stty $(stty -g)" EXIT
stty -echo -icanon -icrnl time 0 min 0

tput clear
tput civis
directionX=-1
directionY=0

yTerminal=$(/bin/stty -a |grep row |awk '{print $5}'|awk -F';' '{print $1}')
xTerminal=$(/bin/stty -a |grep row |awk '{print $7}'|awk -F';' '{print $1}')

bottom=$((yTerminal-2))
top=1

gameEnd=1


ballDirectionX=-1
ballDirectionY=1
ballY=$((yTerminal/2))
ballX=$((xTerminal/2))
ballSpeed=$1

player=$((yTerminal/2))
lastPlayerMove=0

enemy=$((yTerminal/2))
lastEnemyMove=0
enemySpeed=$2

paddleLenght=7

lastTime=$(($(date +%s%N)/1000000))
lastEnemyTime=$(($(date +%s%N)/1000000))


text=''

printArena(){
	echo -en "\033[32m";
	tput cup 0 0
	for (( i = 0; i < $xTerminal; i++ )); do
		echo -n "▬"
	done
	tput cup $yTerminal 0
	for (( i = 0; i < $xTerminal; i++ )); do
		echo -n "▬"
	done
	echo -en "\033[97m";
}

playerMovement(){
	local key=$(cat -v)
	local oldPlayer=$player
	if [ "$key" == "q" ]; then
			gameEnd=0
			text="Only cowards give up"
		fi
	if [ "$key" == "^[[A" ]; then
			player=$((player-1))
	fi

	if [ "$key" == "^[[B" ]; then
			player=$((player+1))
	fi

	if [[ $player -eq 0 ]]; then
		player=$oldPlayer
	elif [[ $((player+paddleLenght)) -gt $((bottom+1)) ]]; then
			player=$oldPlayer
	fi
	lastPlayerMove=$((player-oldPlayer))
}

clearPlayer(){
	if [[ $lastPlayerMove -eq 1 ]]; then
		tput cup $((player-1)) 0
		echo " "
	elif [[ $lastPlayerMove -eq -1 ]]; then
		tput cup $((player+paddleLenght)) 0
		echo " "
	fi

}

printPlayer(){
	tput cup $player 0
	for (( i = 0; i < $paddleLenght; i++ )); do
		echo "|"
	done
}

changeBallDirection(){
	if [[  $ballY -ge $bottom ]]; then
		ballDirectionY=$((ballDirectionY*-1))
		ballY=$bottom
	fi
	if [[ $ballY -le $top ]]; then
		ballDirectionY=$((ballDirectionY*-1))
		ballY=$top
	fi
}

clearBall(){
	tput cup $ballY $ballX
	echo " "	
}

printBall(){
	tput cup $ballY $ballX
	printf "⎕";
}

moveBall(){
	ballX=$((ballX+ballDirectionX))
	ballY=$((ballY+ballDirectionY))
}

ballMovement(){
	time=$(($(date +%s%N)/1000000))
	if [[ $time -gt $lastTime ]]; then
		clearBall
		moveBall
		changeBallDirection
		isHit
		
		if [[ $gameEnd -eq 1 ]]; then
			printBall
		fi
		lastTime=$((time+ballSpeed))
	fi
}

clearEnemy(){
	if [[ $lastEnemyMove -eq 1 ]]; then
		tput cup $((enemy-1)) $xTerminal
		echo " "
	elif [[ $lastEnemyMove -eq -1 ]]; then
		tput cup $((enemy+paddleLenght)) $xTerminal
		echo " "
	fi

}

printEnemy(){
	tput cup $enemy $xTerminal
	for (( i = 0; i < $paddleLenght; i++ )); do
		tput cup $((enemy+i)) $xTerminal
		echo "|"
	done
}

enemyMovement(){
	time=$(($(date +%s%N)/1000000))
	if [[ $time -gt $lastEnemyTime ]]; then
		local oldEnemy=$enemy
		if [ $((enemy+paddleLenght/2)) -lt $ballY ]; then
				enemy=$((enemy+1))
		elif [ $((enemy+paddleLenght/2)) -gt $ballY ]; then
				enemy=$((enemy-1))
		fi

		if [[ $enemy -eq 0 ]]; then
			enemy=$oldEnemy
		elif [[ $((enemy+paddleLenght)) -gt $((bottom+1)) ]]; then
				enemy=$oldEnemy
		fi
		lastEnemyMove=$((enemy-oldEnemy))
		clearEnemy
		printEnemy
		lastEnemyTime=$((time+enemySpeed))
	fi
}

isHit(){
	if [[ $ballX -le 0 ]]; then
		if [[ $player -gt $ballY  ||  $((player+paddleLenght)) -lt $ballY ]]; then
			gameEnd=0
			text="You're a looser"
		else
			ballDirectionX=1
			ballX=0
			advancedHitFor7Paddle $player
			oldTime=$((oldTime-speed))
		fi
	fi

	if [[ $ballX -ge $xTerminal ]]; then
		if [[ $enemy -gt $ballY  ||  $((enemy+paddleLenght)) -lt $ballY ]]; then
			gameEnd=0
			text="You're a great warrior"
		else
			ballDirectionX=-1
			ballx=$xTerminal
			advancedHitFor7Paddle $enemy
			oldTime=$((oldTime-speed))
		fi
	fi
}

advancedHitFor7Paddle(){
	local owner=$1
	local segment=$((owner+paddleLenght-ballY))
	case $segment in
		7)
			ballDirectionY=-3
			;;
		6)
			ballDirectionY=-2
			;;
		5)
			ballDirectionY=-1
			;;
		4)	
			ballDirectionY=0
			;;
		3)
			ballDirectionY=1
			;;
		2)
			ballDirectionY=2
			;;
		1)
			ballDirectionY=3
			;;
	esac 
}

printArena
while [[ $gameEnd -eq 1 ]]; do
	ballMovement
	playerMovement
	clearPlayer
	printPlayer
	enemyMovement
	
done
tput clear
tput cnorm -- normal
echo $text