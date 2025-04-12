#include <fcntl.h>
#include <unistd.h>
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
            write(STDOUT_FILENO, "Invalid input given, position already taken", 43);
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

    // hard part check for other diagonal win
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

idx pickEmptyIdx(idx *idxs) {
    if (idxs != NULL) {
        for (int i = 0; i < boardDim; i++) {
            if (board[idxs[i].x][idxs[i].y] == BOARDNULL) {
                return idxs[i];
            }
        }
    }

    emptyPos empties = getEmptyPos();
    idx pos = empties.pos[random() % empties.len];
    free(empties.pos);
    return pos;
}

idx getWinMove(winInfo *wininfo) {
    if (wininfo == NULL) {
        idx pos = pickEmptyIdx(NULL);
        return pos;
    }

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

    idx emptyPos = pickEmptyIdx(posWinMoves);
    free(posWinMoves);
    return emptyPos;
}

void destroyMatrix(int **matrix, int size) {
    for (int i = 0; i < size; i++) {
        free(matrix[i]);
    }
    free(matrix);
}

void oneMoveFromWinHelper(int playerIdx, winInfo *wininfo, Set *winset, idxNode *headWinPos, 
                            int starti, int endi, int stepi, int startj, int endj, int stepj) {
    int **subMat = calloc((boardDim - 1), sizeof(int *));
    for (int i = 0; i < boardDim - 1; i++) {
        subMat[i] = calloc((boardDim - 1), sizeof(int));
    }

    // populate sub matrix
    int k = 0, h = 0;
    for (int i = starti; i < endi; i+=stepi) {
        h = 0;
        for (int j = startj; j < endj; j+=stepj) {
            subMat[k][h] = board[i][j];
            h++;
        }
        k++;
    }

    wininfo = getWinInfo(subMat, boardDim - 1, playerIdx);
    if (wininfo != NULL) winset = setUnion(winset, wininfo->winset);

    if (headWinPos == NULL) {
        headWinPos = malloc(sizeof(idxNode));
        idx pos = getWinMove(wininfo);
        headWinPos->next = NULL;
        headWinPos->data.x = pos.x;
        headWinPos->data.y = pos.y;
    } else {
        idxNode *temp = headWinPos;
        while(temp->next != NULL) {
            temp = temp->next;
        }

        temp->next = malloc(sizeof(idxNode));
        idx pos = getWinMove(wininfo);
        temp->next->data.x = pos.x;
        temp->next->data.y = pos.y;
    }
    
    if (wininfo != NULL) {
        freeSet(wininfo->winset);
        free(wininfo);
    }
    destroyMatrix(subMat, boardDim - 1);
}

winMove oneMoveFromWin(int playerIdx) {
    Set *wins = createSet(3);
    winInfo *wininfo = malloc(sizeof(winInfo));
    idxNode *head = NULL;

    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 1, boardDim, 1, 
                        1, boardDim, 1);
    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 0, boardDim - 1, 1,
                        0, boardDim - 1, 1);
    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 1, boardDim, 1,
                        0, boardDim - 1, 1);
    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 0, boardDim - 1, 1,
                        1, boardDim, 1);
    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 0, boardDim, 2,
                        0, boardDim - 1, 1);
    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 0, boardDim, 2,
                        1, boardDim, 1);
    oneMoveFromWinHelper(playerIdx, wininfo, wins, head, 0, boardDim, 2,
                        0, boardDim, 2);

    winMove winningMove;
    idxNode *temp = head;
    while (temp != NULL) {
        if (temp->data.x != -1 && temp->data.y != -1) {
            winningMove.move = temp->data;
        }
    }

    if (contains(wins, 1) || contains(wins, 2)) {
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
        return pickEmptyIdx(NULL);
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

        winMove oppwinmove = oneMoveFromWin(2);
        idx pos;
        if (oppwinmove.win) {
            pos = oppwinmove.move;
        } else {
            pos = findDecentMove(1);
        }

        board[pos.x][pos.y] = 1;

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
    playTicTacToe(3);
    for (int i = 0; i < boardDim; i++) {
        free(board[i]);
    }
    free(board);
    return 0;
}