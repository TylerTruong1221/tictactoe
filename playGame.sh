#!/bin/bash

echo "Welcome to tic-tac-toe"
echo "Please choose an option: "
echo "1. Player vs. Bot"
echo "2. Player vs. Player"
echo "3. Bot vs. Bot"
read choice
if [ "$choice" -eq 1 ]; then
	./playerVsBot.sh
elif [ "$choice" -eq 2 ]; then
	./playerVsPlayer.sh
elif [ "$choice" -eq 3 ]; then
	./loop100.sh
else
	echo "Invalid choice, try again"
	./playGame.sh
fi
