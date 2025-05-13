#!/bin/bash

PLAYER_X="/home/TylerTruong1221/tictactoe/playerX.c"
PLAYER_O="/home/TylerTruong1221/tictactoe/playerO.sh"

while true; do
	echo "Player X's turn"
	./playerX
	sleep 1

	echo "Player O's turn"
	bash "$PLAYER_O"
	sleep 1
done
