#include <time.h>
#include "set.h"

#define BUFSIZE 128
#define BOARDNULL 0

static int **board;
static int boardDim;

typedef struct twoTuple {
    int x, y;
} idx;

typedef struct node {
    idx data;
    struct node *next;
} idxNode;

typedef enum { ROW, COL, MAINDIAG, OTHERDIAG } wintype;

typedef struct wininfo {
    wintype type;
    int index;
    Set *winset;
} winInfo;

typedef struct winmove {
    int win;
    idx move;
} winMove;

typedef struct emptypos {
    idx *pos;
    int len;
} emptyPos;

void initBoard(int boardSize) {
    board = calloc(boardSize, sizeof(int *));
    for (int i = 0; i < boardSize; i++) {
        board[i] = calloc(boardSize, sizeof(int));
    }
    boardDim = boardSize;
}

void printBoard() {
    for (int i = 0; i < boardDim; i++) {
        for (int j = 0; j < boardDim; j++) {
            char buf[2];
            buf[0] = board[i][j] + '0';
            buf[1] = '\0';
            write(STDOUT_FILENO, buf, 1);
        }
        write(STDOUT_FILENO, "\n\0", 1);
    }
}

idx *getPosition(int playerIdx) {
    char msg[BUFSIZE] = "Player   input position (e.g., 0,0): \0";
    msg[7] = playerIdx + '0';
    write(STDOUT_FILENO, msg, strlen(msg));
    
    char buf[BUFSIZE];
    idx *tuple = malloc(sizeof(idx));

    if (read(STDIN_FILENO, buf, 4) == 4) {
        // process
        tuple->x = buf[0] - '0';
        tuple->y = buf[2] - '0';
    } else {
        perror("please input board position in index,index format (e.g. 0,0)");
    }

    return tuple;
}

void getApplyInput(int playerIdx) {
    idx *tuple;
    while (1) {
        tuple = getPosition(playerIdx);

        if (board[tuple->x][tuple->y] != BOARDNULL) {
            write(STDOUT_FILENO, "Invalid input given, position already taken\n", 44);
            continue;
        }

        if (board[tuple->x][tuple->y] == BOARDNULL) break;
    }

    board[tuple->x][tuple->y] = playerIdx;
}

winInfo *getWinInfo(int **currBoard, int currBoardSize, int playerIdx) {
    Set *winset = createSet(3);
    winInfo *wininfo = malloc(sizeof(winInfo));
    wininfo->winset = winset;
    int otherPlayerIdx = playerIdx == 1 ? 2 : 1;

    for (int i = 0; i < currBoardSize; i++) {
        // curr row
        for (int j = 0; j < currBoardSize; j++) {
            add(winset, currBoard[i][j]);
        }

        if (!contains(winset, 0) && !contains(winset, otherPlayerIdx) && contains(winset, playerIdx)) {
            // processing
            wininfo->type = ROW;
            wininfo->index = i;
            return wininfo;
        }

        // empty winset
        removeValue(winset, 0);
        removeValue(winset, 1);
        removeValue(winset, 2);

        // curr col
        for (int k = 0; k < currBoardSize; k++) {
            add(winset, currBoard[k][i]);
        }

        if (!contains(winset, 0) && !contains(winset, otherPlayerIdx) && contains(winset, playerIdx)) {
            // processing
            wininfo->type = COL;
            wininfo->index = i;
            return wininfo;
        }

        // empty winset
        removeValue(winset, 0);
        removeValue(winset, 1);
        removeValue(winset, 2);
    }

    for (int i = 0; i < currBoardSize; i++) {
        add(winset, currBoard[i][i]);
    }

    if (!contains(winset, 0) && !contains(winset, otherPlayerIdx) && contains(winset, playerIdx)) {
        // processing
        wininfo->type = MAINDIAG;
        wininfo->index = -1;
        return wininfo;
    }

    // empty winset
    removeValue(winset, 0);
    removeValue(winset, 1);
    removeValue(winset, 2);

    for (int i = 0; i < currBoardSize; i++) {
        add(winset, currBoard[i][currBoardSize - 1 - i]);
    }

    if (!contains(winset, 0) && !contains(winset, otherPlayerIdx) && contains(winset, playerIdx)) {
        // processing
        wininfo->type = OTHERDIAG;
        wininfo->index = -1;
        return wininfo;
    }

    return NULL;
}

emptyPos getEmptyPos() {
    emptyPos emptypos;
    emptypos.pos = malloc(boardDim * boardDim * sizeof(idx));
    int k = 0;
    for (int i = 0; i < boardDim; i++) {
        for (int j = 0; j < boardDim; j++) {
            if (board[i][j] == BOARDNULL) {
                idx pos;
                pos.x = i;
                pos.y = j;
                emptypos.pos[k++] = pos;
            }
        }
    }

    emptypos.len = k;
    return emptypos;
}

idx pickEmptyIdx(idx *idxs, int pickBoardEmpty) {
    if (idxs != NULL) {
        for (int i = 0; i < boardDim; i++) {
            if (board[idxs[i].x][idxs[i].y] == BOARDNULL) {
                return idxs[i];
            }
        }
    }

    if (pickBoardEmpty) {
        emptyPos empties = getEmptyPos();
        idx pos = empties.pos[rand() % empties.len];
        free(empties.pos);
        return pos;
    }

    idx pos;
    pos.x = -1;
    pos.y = -1;
    return pos;
}

idx getWinMove(winInfo *wininfo) {
    idx *posWinMoves = malloc(boardDim * sizeof(idx));

    if (wininfo->type == ROW) {
        for (int i = 0; i < boardDim; i++) {
            posWinMoves[i].x = wininfo->index;
            posWinMoves[i].y = i;
        }
    } else if (wininfo->type == COL) {
        for (int i = 0; i < boardDim; i++) {
            posWinMoves[i].x = i;
            posWinMoves[i].y = wininfo->index;
        }
    } else if (wininfo->type == MAINDIAG) {
        for (int i = 0; i < boardDim; i++) {
            posWinMoves[i].x = i;
            posWinMoves[i].y = i;
        }
    } else if (wininfo->type == OTHERDIAG) {
        for (int i = 0; i < boardDim; i++) {
            posWinMoves[i].x = i;
            posWinMoves[i].y = boardDim - 1 - i;
        }
    }

    idx emptyPos = pickEmptyIdx(posWinMoves, 0);
    free(posWinMoves);
    return emptyPos;
}

void destroyMatrix(int **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

winMove oneMoveFromWin(int playerIdx) {
    Set *wins = createSet(3);
    idx *pos = calloc(7, sizeof(idx));
    int posPos = 0;

    for (int i = 0; i < 7; i++) {
        pos[i].x = -1;
        pos[i].y = -1;
    }

    int **subMat1 = calloc((boardDim - 1), sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat1[i] = calloc((boardDim - 1), sizeof(int));
    }

    // populate sub matrix
    // otherdiag doesn't constitute win
    int k = 0, h = 0;
    for (int i = 1; i < boardDim; i++) {
        h = 0;
        for (int j = 1; j < boardDim; j++) {
            subMat1[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    winInfo *wininfo1 = getWinInfo(subMat1, boardDim - 1, playerIdx);

    // see line 259
    if (wininfo1 != NULL && wininfo1->type == OTHERDIAG) {
        freeSet(wininfo1->winset);
        free(wininfo1);
        wininfo1 = NULL;
    } else if (wininfo1 != NULL) {
        // account for mismatching indicies in 2x2 and 3x3 boards
        if (wininfo1->type == COL || wininfo1->type == ROW) {
            wininfo1->index += 1;
        }

        wins = setUnion(wins, wininfo1->winset);
        pos[posPos] = getWinMove(wininfo1);
    }
    posPos++;
    destroyMatrix(subMat1, boardDim - 1);

    int **subMat2 = malloc((boardDim - 1) * sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat2[i] = malloc((boardDim - 1) * sizeof(int));
    }

    // populate sub matrix
    // otherdiag doesn't constitute win
    for (int i = 0; i < boardDim - 1; i++) {
        for (int j = 0; j < boardDim - 1; j++) {
            subMat2[i][j] = board[i][j];
        }
    }

    winInfo *wininfo2 = getWinInfo(subMat2, boardDim - 1, playerIdx);

    // see line 286
    if (wininfo2 != NULL && wininfo2->type == OTHERDIAG) {
        freeSet(wininfo2->winset);
        free(wininfo2);
        wininfo2 = NULL;
    } else if (wininfo2 != NULL) {
        wins = setUnion(wins, wininfo2->winset);
        pos[posPos] = getWinMove(wininfo2);
    }
    posPos++;
    destroyMatrix(subMat2, boardDim - 1);

    int **subMat3 = malloc((boardDim - 1) * sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat3[i] = malloc((boardDim - 1) * sizeof(int));
    }

    // populate sub matrix
    // maindiag doesn't constitute win
    k = 0, h = 0;
    for (int i = 1; i < boardDim; i++) {
        h = 0;
        for (int j = 0; j < boardDim - 1; j++) {
            subMat3[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    winInfo *wininfo3 = getWinInfo(subMat3, boardDim - 1, playerIdx);

    // see line 309
    if (wininfo3 != NULL && wininfo3->type == MAINDIAG) {
        freeSet(wininfo3->winset);
        free(wininfo3);
        wininfo3 = NULL;
    } else if (wininfo3 != NULL) {
        if (wininfo3->type == ROW) {
            wininfo3->index += 1;
        }

        wins = setUnion(wins, wininfo3->winset);
        pos[posPos] = getWinMove(wininfo3);
    }
    posPos++;
    destroyMatrix(subMat3, boardDim - 1);

    int **subMat4 = malloc((boardDim - 1) * sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat4[i] = malloc((boardDim - 1) * sizeof(int));
    }

    // populate sub matrix
    // maindiag doesn't constitute win
    k = 0, h = 0;
    for (int i = 0; i < boardDim - 1; i++) {
        h = 0;
        for (int j = 1; j < boardDim; j++) {
            subMat4[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    winInfo *wininfo4 = getWinInfo(subMat4, boardDim - 1, playerIdx);

    // see line 336
    if (wininfo4 != NULL && wininfo4->type == MAINDIAG) {
        freeSet(wininfo4->winset);
        free(wininfo4);
        wininfo4 = NULL;
    } else if (wininfo4 != NULL) {
        if (wininfo4->type == COL) {
            wininfo4->index += 1;
        }

        wins = setUnion(wins, wininfo4->winset);
        pos[posPos] = getWinMove(wininfo4);
    }
    posPos++;
    destroyMatrix(subMat4, boardDim - 1);

    int **subMat5 = malloc((boardDim - 1) * sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat5[i] = malloc((boardDim - 1) * sizeof(int));
    }

    // populate sub matrix
    // maindiag and otherdiag don't constitute wins
    k = 0, h = 0;
    for (int i = 0; i < boardDim; i+=2) {
        h = 0;
        for (int j = 0; j < boardDim - 1; j++) {
            subMat5[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    winInfo *wininfo5 = getWinInfo(subMat5, boardDim - 1, playerIdx);

    // see line 363
    if (wininfo5 != NULL && (wininfo5->type == MAINDIAG || wininfo5->type == OTHERDIAG)) {
        freeSet(wininfo5->winset);
        free(wininfo5);
        wininfo5 = NULL;
    } else if (wininfo5 != NULL) {
        if (wininfo5->type == ROW && wininfo5->index == 1) {
            wininfo5->index += 1;
        }

        wins = setUnion(wins, wininfo5->winset);
        pos[posPos] = getWinMove(wininfo5);
    }
    posPos++;
    destroyMatrix(subMat5, boardDim - 1);

    int **subMat6 = malloc((boardDim - 1) * sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat6[i] = malloc((boardDim - 1) * sizeof(int));
    }

    k = 0, h = 0;
    // populate sub matrix
    // maindiag and otherdiag don't constitute wins
    for (int i = 0; i < boardDim; i+=2) {
        h = 0;
        for (int j = 1; j < boardDim; j++) {
            subMat6[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    winInfo *wininfo6 = getWinInfo(subMat6, boardDim - 1, playerIdx);

    // see line 392
    if (wininfo6 != NULL && (wininfo6->type == MAINDIAG || wininfo6->type == OTHERDIAG)) {
        freeSet(wininfo6->winset);
        free(wininfo6);
        wininfo6 = NULL;
    } else if (wininfo6 != NULL) {
        if ((wininfo6->type == ROW && wininfo6->index == 1) || wininfo6->type == COL) {
            wininfo6->index += 1;
        }

        wins = setUnion(wins, wininfo6->winset);
        pos[posPos] = getWinMove(wininfo6);
    }
    posPos++;
    destroyMatrix(subMat6, boardDim - 1);

    int **subMat7 = malloc((boardDim - 1) * sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat7[i] = malloc((boardDim - 1) * sizeof(int));
    }

    k = 0, h = 0;
    // populate sub matrix
    // everything constitutes a win
    for (int i = 0; i < boardDim; i+=2) {
        h = 0;
        for (int j = 0; j < boardDim; j+=2) {
            subMat7[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    winInfo *wininfo7 = getWinInfo(subMat7, boardDim - 1, playerIdx);
    if (wininfo7 != NULL) {
        if ((wininfo7->type == ROW && wininfo7->index == 1) || (wininfo7->type == COL && wininfo7->index == 1)) {
            wininfo7->index += 1;
        }

        wins = setUnion(wins, wininfo7->winset);
        pos[posPos] = getWinMove(wininfo7);
    }
    posPos++;
    destroyMatrix(subMat7, boardDim - 1);

    winMove winningMove;
    for (int i = 0; i < posPos; i++) {
        if (pos[i].x != -1 && pos[i].y != -1) {
            winningMove.move = pos[i];
        }
    }

    // free all allocated memory
    if (wininfo1 != NULL) {
        freeSet(wininfo1->winset);
        free(wininfo1);
    }
    if (wininfo2 != NULL) {
        freeSet(wininfo2->winset);
        free(wininfo2);
    }
    if (wininfo3 != NULL) {
        freeSet(wininfo3->winset);
        free(wininfo3);
    }
    if (wininfo4 != NULL) {
        freeSet(wininfo4->winset);
        free(wininfo4);
    }
    if (wininfo5 != NULL) {
        freeSet(wininfo5->winset);
        free(wininfo5);
    }
    if (wininfo6 != NULL) {
        freeSet(wininfo6->winset);
        free(wininfo6);
    }
    if (wininfo7 != NULL) {
        freeSet(wininfo7->winset);
        free(wininfo7);
    }

    if (contains(wins, playerIdx)) {
        winningMove.win = 1;
        return winningMove;
    } else {
        winningMove.win = 0;
        return winningMove;
    }
}

idx findDecentMove(int playerIdx) {
    winMove winningMove = oneMoveFromWin(playerIdx);

    if (winningMove.win) {
        return winningMove.move;
    } else {
        return pickEmptyIdx(NULL, 1);
    }
}

int boardFull() {
    for (int i = 0; i < boardDim; i++) {
        for (int j = 0; j < boardDim; j++) {
            if (board[i][j] == BOARDNULL)
                return 0;
        }
    }

    return 1;
}

void playTicTacToe(int boardSize) {
    initBoard(boardSize);
    printBoard();

    winInfo *wininfo1;
    winInfo *wininfo2;
    while (1) {
        write(STDOUT_FILENO, "Computer (Player 1) makes move:\n", 32);

        winMove compwinmove = oneMoveFromWin(1);
        winMove oppwinmove = oneMoveFromWin(2);
        idx pos;
        if (compwinmove.win) {
            pos = compwinmove.move;
        } else if (oppwinmove.win) {
            pos = oppwinmove.move;
        } else {
            pos = findDecentMove(1);
        }

        if (board[pos.x][pos.y] == BOARDNULL)
            board[pos.x][pos.y] = 1;
        else {
            if (oppwinmove.win && board[oppwinmove.move.x][oppwinmove.move.y] == BOARDNULL) pos = oppwinmove.move;
            else pos = pickEmptyIdx(NULL, 1);
            board[pos.x][pos.y] = 1;
        }

        printBoard();

        wininfo1 = getWinInfo(board, boardDim, 1);

        if (wininfo1 != NULL && (contains(wininfo1->winset, 1))) {
            write(STDOUT_FILENO, "Player 1 wins!", 15);
            free(wininfo1->winset);
            free(wininfo1);
            break;
        }

        if (wininfo1 != NULL) {
            free(wininfo1->winset);
            free(wininfo1);
        }

        if (boardFull()) {
            write(STDOUT_FILENO, "Tie!", 5);
            break;
        }
        getApplyInput(2);

        printBoard();

        wininfo2 = getWinInfo(board, boardDim, 2);

        if (wininfo2 != NULL && (contains(wininfo2->winset, 2))) {
            write(STDOUT_FILENO, "Player 2 wins!", 15);
            free(wininfo2->winset);
            free(wininfo2);
            break;
        }

        if (wininfo2 != NULL) {
            free(wininfo2->winset);
            free(wininfo2);
        }

        if (boardFull()) {
            write(STDOUT_FILENO, "Tie!", 5);
            break;
        }
    }
}

int main(int argc, char **argv) {
    srand(time(0));
    playTicTacToe(3);
    for (int i = 0; i < boardDim; i++) {
        free(board[i]);
    }
    free(board);
    return 0;
}