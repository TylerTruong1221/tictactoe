#!/bin/bash

PLAYER="X"
TOPIC="player1/move"
BROKER="35.227.189.114"
BOARD_FILE="board.txt"

# initialize board
if [ ! -f "$BOARD_FILE" ] || [ $(wc -c < "$BOARD_FILE") -ne 9 ]; then
    echo "_________" > "$BOARD_FILE"
fi

mosquitto_sub -h "$BROKER" -t "tictactoe/board" -C 1 > "$BOARD_FILE"

if [ ! -f "$BOARD_FILE" ]; then
	echo "Error: board.txt not found"
	exit 1
fi

board=$(cat "$BOARD_FILE")

empty_positions=()
for i in {0..8}; do
    char="${board:$i:1}"
    if [[ "$char" != "X" && "$char" != "O" ]]; then
        empty_positions+=($((i+1)))
    fi
done

if [ ${#empty_positions[@]} -eq 0 ]; then
    echo "No moves left."
    exit 0
fi

while true; do
    read -p "Player $PLAYER, enter your move (${empty_positions[*]}): " move
    if [[ ! "$move" =~ ^[1-9]$ ]]; then
        echo "Invalid input. Please enter a number between 1 and 9."
        ./player.sh
    fi
    if [[ ! " ${empty_positions[@]} " =~ " $move " ]]; then
        echo "Position $move is already taken or invalid. Choose from: ${empty_positions[*]}"
        ./player.sh
    fi
    break
done

mosquitto_pub -h "$BROKER" -t "$TOPIC" -m "$move"

pos=$((move - 1))
new_board="${board:0:$pos}$PLAYER${board:$((pos + 1))}"
echo "$new_board" > "$BOARD_FILE"

echo "Player $PLAYER moved to position $move"

board=$(cat "$BOARD_FILE")
echo "${board:0:1}|${board:1:1}|${board:2:1} "
echo "${board:3:1}|${board:4:1}|${board:5:1} "
echo "${board:6:1}|${board:7:1}|${board:8:1} "

./check_results.sh
