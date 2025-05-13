#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define PLAYER 'X'
#define BOARD_FILE "board.txt"
#define TOPIC "player1/move"
#define BROKER "35.227.189.114"

void initializeBoard(char *board) {
    FILE *fp = fopen(BOARD_FILE, "r");
    if (!fp || fgetc(fp) == EOF) {
        fclose(fp);
        fp = fopen(BOARD_FILE, "w");
        fprintf(fp, "_________");
        fclose(fp);
    } else {
        fclose(fp);
    }
}

void updateBoardFromBroker() {
    system("mosquitto_sub -h "BROKER" -t tictactoe/board -C 1 > board.txt");
}

int readBoard(char *board) {
    FILE *fp = fopen(BOARD_FILE, "r");
    if (!fp) {
        fprintf(stderr, "Error: %s not found\n", BOARD_FILE);
        return 0;
    }
    fread(board, 1, 9, fp);
    board[9] = '\0';
    fclose(fp);
    return 1;
}

void printBoard(const char *board) {
    printf("%c|%c|%c\n", board[0], board[1], board[2]);
    printf("%c|%c|%c\n", board[3], board[4], board[5]);
    printf("%c|%c|%c\n", board[6], board[7], board[8]);
}

int isValidMove(int move, int *empty, int count) {
    for (int i = 0; i < count; i++) {
        if (empty[i] == move) return 1;
    }
    return 0;
}

int main() {
    char board[10];
    int empty[9], count = 0;
    char moveStr[10];
    int move, valid = 0;

    initializeBoard(board);
    updateBoardFromBroker();
    if (!readBoard(board)) return 1;

    // Find empty positions
    for (int i = 0; i < 9; i++) {
        if (board[i] != 'X' && board[i] != 'O') {
            empty[count++] = i + 1;
        }
    }

    if (count == 0) {
        printf("No moves left.\n");
        return 0;
    }

    // Prompt user for move
    do {
        printf("Player %c, enter your move (", PLAYER);
        for (int i = 0; i < count; i++) {
            printf("%d%s", empty[i], (i < count - 1) ? ", " : "");
        }
        printf("): ");
        fgets(moveStr, sizeof(moveStr), stdin);
        move = atoi(moveStr);

        if (move >= 1 && move <= 9 && isValidMove(move, empty, count)) {
            valid = 1;
        } else {
            printf("Invalid move. Try again.\n");
        }
    } while (!valid);

    // Publish move
    char pubCmd[100];
    snprintf(pubCmd, sizeof(pubCmd), "mosquitto_pub -h %s -t %s -m %d", BROKER, TOPIC, move);
    system(pubCmd);

    // Update board
    board[move - 1] = PLAYER;
    FILE *fp = fopen(BOARD_FILE, "w");
    if (fp) {
        fwrite(board, 1, 9, fp);
        fclose(fp);
    }

    printf("Player %c moved to position %d\n", PLAYER, move);
    printBoard(board);

    system("./check_results.sh");

    return 0;
}
