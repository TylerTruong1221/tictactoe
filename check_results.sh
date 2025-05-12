#!/bin/bash

BROKER="35.227.189.114"
TOPIC="tictactoe/gameStatus"

mosquitto_sub -h "$BROKER" -t "$TOPIC" -C 1 | while read -r status; do
    if [ "$status" != "Ongoing" ]; then
        echo "Game Status: $status"
    fi
done
