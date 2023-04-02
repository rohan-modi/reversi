#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>

enum STYLES {
  DEFAULT=0,
  CLEAN=1
};

const int MAX_DEPTH=2;

bool printStuff = false;
enum STYLES boardStyle = CLEAN;
int totalMovesCounter = 1;

struct rusage usage; // a structure to hold "resource usage" (including time)
struct timeval start, end; // will hold the start and end times
double timeStart, timeEnd, totalTime;

bool isOccupied(char board[][26], int row, int col) {
    return board[row][col] == 'U';
}

void printBoard(char board[][26], int n) {
    printf("  ");
    char letter =  'a';
    for (int i = 0; i < n; i++) {
        printf("%c", letter);
        letter++;
    }
    letter = 'a';
    for (int i = 0; i < n; i++) {
        printf("\n%c ", letter);
        letter++;
        for (int j = 0; j < n; j++) {
            printf("%c", board[i][j]);
        }
    }
}

bool positionInBounds(int n, int row, int col) {
    if (row < n && col < n && row > -1 && col > -1) {
        return true;
    }
    return false;
}

bool checkLegalInDirection(char board[][26], int n, int row, int col, char colour, int deltaRow, int deltaCol) {
    if (!isOccupied(board, row, col)) {
        return false;
    }
    row += deltaRow;
    col += deltaCol;
    if (positionInBounds(n, row, col)) {
        if (board[row][col] == colour || isOccupied(board, row, col)) {
            return false;
        }
    }
    else {
        return false;
    }
    while (1) {
        row += deltaRow;
        col += deltaCol;
        if (positionInBounds(n, row, col)) {
            if (isOccupied(board, row, col)) {
                return false;
            }
        } else {
            return false;
        }
        if (board[row][col] == colour) {
            return true;
        }
    }
}

int getMoveScore(char board[][26], int n, char colour, int row, int col) {
    int startRow = row;
    int startCol = col;
    int score = 0;
    bool edgeRow, edgeCol;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            row = startRow;
            col = startCol;
            if (checkLegalInDirection(board, n, row, col, colour, i, j)) {
                while (1) {
                    row += i;
                    col += j;
                    if (positionInBounds(n, row, col)) {
                        if (isOccupied(board, row, col)) {
                            break;
                        }
                        if (!isOccupied(board, row, col) && board[row][col] != colour) {
                            score++;
                            edgeRow = false;
                            edgeCol = false;
                            if (row == 0 || row == n-1) {
                                edgeRow = true;
                            }
                            if (col == 0 || col == n-1) {
                                edgeCol = true;
                            }
                            if (edgeRow || edgeCol) {
                                score += 1;
                                if (edgeRow && edgeCol) {
                                    score += 3;
                                }
                            }
                        }
                        else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }
    return score;
}

void getBoardScores(char board[][26], int n, char colour, int scores[][26]) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scores[i][j] = getMoveScore(board, n, colour, i, j);
        }
    }
}

void copyBoard(char original[][26], char copy[][26], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            copy[i][j] = original[i][j];
        }
    }
}

bool hasLegalMove(char board[][26], int n, char colour) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            for (int k = -1; k < 2; k++) {
                for (int l = -1; l < 2; l++) {
                    if (k == 0 && l == 0) {
                        continue;
                    }
                    if (checkLegalInDirection(board, n, i, j, colour, k, l)) {
                        return true;
                    }
                }
            }
        }
    }
    return false;
}

void flipPieces(char board[][26], int n, char move[4]) {
    int currentRow = move[1] - 'a';
    int currentCol = move[2] - 'a';
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            currentRow = move[1] - 'a';
            currentCol = move[2] - 'a';
            if (checkLegalInDirection(board, n, currentRow, currentCol, move[0], i, j)) {
                while (1) {
                    currentRow += i;
                    currentCol += j;
                    if (positionInBounds(n, currentRow, currentCol)) {
                        if (board[currentRow][currentCol] == 'U') {
                            break;
                        }
                        if (board[currentRow][currentCol] != 'U' && board[currentRow][currentCol] != move[0]) {
                            board[currentRow][currentCol] = move[0];
                        }
                        else {
                            break;
                        }
                    } else {
                        break;
                    }
                }
            }
        }
    }
    currentRow = move[1] - 'a';
    currentCol = move[2] - 'a';
    board[currentRow][currentCol] = move[0];
}

int findMove(char board[][26], int n, char turn, int *row, int *col, int maxDepth) {
    int bestScore = -1000, tempScore, additionalScore;
    int bestRow, bestCol;
    char nextBoard[26][26];
    char otherColour = 'B';
    char move[4];
    if (turn == 'B') {
        otherColour = 'W';
    }
    int scores[26][26];
    getBoardScores(board, n, turn, scores);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scores[i][j] > bestScore) {
                bestScore = scores[i][j];
                (*row) = i;
                (*col) = j;
            }
        }
    }
    // for (int i = 0; i < n; i++) {
    //     for (int j = 0; j < n; j++) {
    //         tempScore = getMoveScore(board, n, turn, i, j);
    //         if (tempScore == 0) {
    //             continue;
    //         }
    //         copyBoard(board, nextBoard, n);
    //         move[0] = turn;
    //         move[1] = i + 'a';
    //         move[2] = j + 'a';
    //         flipPieces(nextBoard, n, move);
    //         if (maxDepth>0) {
    //             additionalScore = findMove(nextBoard, n, otherColour, row, col, maxDepth-1);
    //             tempScore = tempScore-additionalScore;
    //         }
    //         if (tempScore > bestScore) {
    //             bestScore = tempScore + additionalScore;
    //             if (maxDepth == MAX_DEPTH) {
    //                 (*row) = i;
    //                 (*col) = j;
    //             }
    //         }
    //     }
    // }
    return bestScore;
}

int makeMove(char board[][26], int n, char turn, int *row, int *col) {
    return findMove(board, n, turn, row, col, 2);
}

char winner(char board[][26], int n) {
    int scoreB = 0;
    int scoreW = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (board[i][j] == 'B') {
                scoreB++;
            } else if (board[i][j] == 'W') {
                scoreW++;
            }
        }
    }
    if (scoreB > scoreW) {
        return 'B';
    } else if (scoreW > scoreB) {
        return 'W';
    }
    return 'D';
}

int main(void) {
    int size;
    printf("Enter the board dimension: ");
    scanf("%d", &size);
    char board[26][26];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if ((i == size/2 - 1 && j == size/2 - 1) || (i == size/2 && j == size/2)) {
                board[i][j] = 'W';
            } 
            else if ((i == size/2 - 1 && j == size/2) || (i == size/2 && j == size/2 - 1)) {
                board[i][j] = 'B';
            } else {
                board[i][j] = 'U';
            }
        }
    }
    printf("Computer plays (B/W): ");
    char computerColour, playerColour;
    scanf(" %c", &computerColour);
    int bestRow, bestCol;
    char move[4];
    char input[3];
    printBoard(board, size);

    char winningColour;
    int inputRow, inputCol;

    if (computerColour == 'B') {
        playerColour = 'W';
        move[0] = 'B';

        getrusage(RUSAGE_SELF, &usage);
        start = usage.ru_utime;
        timeStart = start.tv_sec + start.tv_usec / 1000000.0; // in seconds

        findMove(board, size, 'B', &bestRow, &bestCol, MAX_DEPTH);
        move[1] = bestRow + 'a';
        move[2] = bestCol + 'a';
        flipPieces(board, size, move);
        printf("\nComputer places B at %c%c.\n", move[1], move[2]);
        printBoard(board, size);
    } 
    else {
        playerColour = 'B';
    }
    while(1) {
        if (hasLegalMove(board, size, playerColour)) {
            printf("\nEnter move for colour %c (RowCol): ", playerColour);
            
            scanf("%s", input);
            move[0] = playerColour;
            move[1] = input[0];
            move[2] = input[1];
            bool validMove = false;
            for (int i = -1; i < 2; i++) {
                for (int j = -1; j < 2; j++) {
                    if (i == 0 && j == 0) {
                        continue;
                    }
                    if (checkLegalInDirection(board, size, move[1] - 'a', move[2] - 'a', playerColour, i, j)) {
                        validMove = true;
                    }
                }
            }
            if (validMove) {
                flipPieces(board, size, move);
                printBoard(board, size);
            } else {
                printf("Invalid move.\n%c player wins.", computerColour);
                return 0;
            }
        } else {
            printf("%c player has no valid move.\n", playerColour);
        }
        if (!hasLegalMove(board, size, computerColour) && !hasLegalMove(board, size, playerColour)) {
            winningColour = winner(board, size);
            if (winningColour == 'D') {
                printf("You both suck");
            } else {
                printf("\n%c player wins.", winningColour);
            }
            return 0;
        }
        move[0] = computerColour;
        if (hasLegalMove(board, size, computerColour)) {

            getrusage(RUSAGE_SELF, &usage);
            start = usage.ru_utime;
            timeStart = start.tv_sec + start.tv_usec / 1000000.0; // in seconds

            findMove(board, size, computerColour, &bestRow, &bestCol, MAX_DEPTH);
            move[1] = bestRow + 'a';
            move[2] = bestCol + 'a';
            flipPieces(board, size, move);
            printf("\nComputer places %c at %c%c.\n", computerColour, move[1], move[2]);
            totalMovesCounter++;
            printBoard(board, size);
        } else {
            printf("%c player has no valid move.\n", computerColour);
        }
        if (!hasLegalMove(board, size, computerColour) && !hasLegalMove(board, size, playerColour)) {
            winningColour = winner(board, size);
            if (winningColour == 'D') {
                printf("You both suck");
            } else {
                printf("\n%c player wins.", winningColour);
            }
            return 0;
        }
    }
    return 0;
}