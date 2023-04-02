#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <sys/time.h>
#include <sys/resource.h>

const int MAX_DEPTH = 10;
const int MAX_TIME = 900;
const int INVALID_MOVE_SCORE = -100000;

const bool PRINT_FRIENDLY_BOARD=true;
const char BLACK = '#';
const char WHITE = 'O';
const char BLANK = ' ';

// const bool PRINT_FRIENDLY_BOARD=false;
// const char BLACK = 'B';
// const char WHITE = 'W';
// const char BLANK = 'U';
// const bool COMPUTER_PLAYS_FOR_HUMAN=false;

struct Score {
    int blackScore;
    int whiteScore;
    char winner;
};

typedef struct Score Score;

bool printStuff = false;
int totalMovesCounter = 1;

bool isOccupied(char board[][26], int row, int col) {
    return board[row][col] == BLANK;
}

void printScores(int board[][26], int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            printf("%d ", board[i][j]);
        }
        printf("\n");
    }
}

// The following function prints the game board along with the axis labeled a-z
// depending on the size of the of the dimensions
void printBoardStyled(char board[][26], int n) {
  // Print the column labels
  printf("  ");
  for (int i = 0; i < n; i++) {
    printf("  %c ", 'a' + i);
  }
  printf("\n  ");
  for (int j = 0; j < n; j++) {
    printf("+---");
  }
  printf("+\n");

  // Print the rows of the board
  for (int i = 0; i < n; i++) {
    printf("%c ", 'a' + i);
    for (int j = 0; j < n; j++) {
      printf("| %c ", board[i][j]);
    }
    printf("|\n");
    printf("  ");
    for (int j = 0; j < n; j++) {
      printf("+---");
    }
    printf("+\n");
  }
}

void printBoard(char board[][26], int n) {
    if (PRINT_FRIENDLY_BOARD) {
        printBoardStyled(board,n);
        return;
    }
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

// Calculates and returns the score for a single cell.
// Does not recurse into further depths.
// Returns -100000 if the move is invalid.
int getMoveScore(char board[][26], int n, char colour, int row, int col) {
    int startRow = row;
    int startCol = col;
    int score = INVALID_MOVE_SCORE;
    bool moveLegal = false;
    bool edgeRow, edgeCol;
    for (int i = -1; i < 2; i++) {
        for (int j = -1; j < 2; j++) {
            row = startRow;
            col = startCol;
            if (checkLegalInDirection(board, n, row, col, colour, i, j)) {
                if (!moveLegal) {
                    score = 0;
                    moveLegal = true;
                }
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

// Calculates the scores for all cells and populates them into the provided 2D array.
// Does not recurse into further depths.
// Cells with no valid move will contain -100000.
int getBoardScores(char board[][26], int n, char colour, int scores[][26]) {
    int counter = 0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scores[i][j] = getMoveScore(board, n, colour, i, j);
            if (scores[i][j] != INVALID_MOVE_SCORE) {
                counter++;
            }
        }
    }
    return counter;
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
                        if (board[currentRow][currentCol] == BLANK) {
                            break;
                        }
                        if (board[currentRow][currentCol] != BLANK && board[currentRow][currentCol] != move[0]) {
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

// Finds the best move for the given board state and sets the row/col to 
// the cell with the highest score (the best move).
// The highest score is returned from the function.
// Board state is not modified by this function.
int findMove(char board[][26], int n, char turn, int *row, int *col, int maxDepth, int maxTime) {

    // start timer here
    struct rusage usage; // a structure to hold "resource usage" (including time)
    struct timeval start, end; // will hold the start and end times
    getrusage(RUSAGE_SELF, &usage);
    start = usage.ru_utime;
    double timeStart = start.tv_sec + start.tv_usec / 1000000.0; // in seconds

    int bestScore = INVALID_MOVE_SCORE, tempScore, additionalScore;
    int bestRow, bestCol;
    char nextBoard[26][26];
    char otherColour = BLACK;
    char move[4];
    if (turn == BLACK) {
        otherColour = WHITE;
    }
    int scores[26][26];
    int numOfValidMoves = getBoardScores(board, n, turn, scores);

    int nextScore;

    // figure out the number of valid next moves

    // printf("\nThe board scores are: \n");
    // printScores(scores, n);
    // printf("\n");

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scores[i][j] == INVALID_MOVE_SCORE) {
                continue;
            }
            if (maxDepth > 0) {
                copyBoard(board, nextBoard, n);
                move[0] = turn;
                move[1] = i + 'a';
                move[2] = j + 'a';
                flipPieces(nextBoard, n, move);

                // Pass time slice to next findMove check
                nextScore = findMove(nextBoard, n, otherColour, row, col, maxDepth - 1, maxTime/numOfValidMoves);
                if (nextScore == INVALID_MOVE_SCORE) {
                    nextScore = 0;
                }
                scores[i][j] = scores[i][j] - nextScore;

                // Check time elapsed, if past maxTime break the loop
                getrusage(RUSAGE_SELF, &usage);
                end = usage.ru_utime;
                double timeEnd = end.tv_sec + end.tv_usec / 1000000.0; // in seconds
                double totalTime = timeEnd - timeStart;
                if (totalTime >= maxTime) {
                    goto afterForLoop;
                }
            }
        }
    }
    afterForLoop:

    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (scores[i][j] > bestScore) {
                bestScore = scores[i][j];
                if (maxDepth == MAX_DEPTH) {
                    // printf("Final scores:\n");
                    // printScores(scores, n);
                    // printf("\n");
                    // printf("COMPUTER FINDING NEW MOVE\n");
                    (*row) = i;
                    (*col) = j;
                }
            }
        }
    }
    return bestScore;
}

// Called by external test program
int makeMove(char board[][26], int n, char turn, int *row, int *col) {
    return findMove(board, n, turn, row, col, MAX_DEPTH, 900);
}

Score winner(char board[][26], int n) {
    Score result;
    result.blackScore=0;
    result.whiteScore=0;
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (board[i][j] == BLACK) {
                result.blackScore++;
            } else if (board[i][j] == WHITE) {
                result.whiteScore++;
            }
        }
    }
    if (result.blackScore > result.whiteScore) {
        result.winner=BLACK;
    } else if (result.whiteScore > result.blackScore) {
        result.winner=WHITE;
    } else {
        result.winner='D';
    }
    return result;
}

int main(void) {
    int size;
    printf("Enter the board dimension: ");
    scanf("%d", &size);
    char board[26][26];
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if ((i == size/2 - 1 && j == size/2 - 1) || (i == size/2 && j == size/2)) {
                board[i][j] = WHITE;
            } 
            else if ((i == size/2 - 1 && j == size/2) || (i == size/2 && j == size/2 - 1)) {
                board[i][j] = BLACK;
            } else {
                board[i][j] = BLANK;
            }
        }
    }
    printf("Computer plays (B/W): ");
    char computerColour, playerColour, inputColour;
    scanf(" %c", &inputColour);

    if (inputColour == 'w' || inputColour == 'W') {
        computerColour = WHITE;
        playerColour = BLACK;
    } else {
        computerColour = BLACK;
        playerColour = WHITE;
    }

    int bestRow, bestCol;
    char move[4];
    char input[3];
    printBoard(board, size);

    Score result;
    int inputRow, inputCol;

    if (computerColour == BLACK) {
        move[0] = BLACK;

        getrusage(RUSAGE_SELF, &usage);
        start = usage.ru_utime;
        timeStart = start.tv_sec + start.tv_usec / 1000000.0; // in seconds

        findMove(board, size, BLACK, &bestRow, &bestCol, MAX_DEPTH, MAX_TIME);
        move[1] = bestRow + 'a';
        move[2] = bestCol + 'a';
        flipPieces(board, size, move);
        printf("\nComputer places %c at %c%c.\n", BLACK, move[1], move[2]);
        printBoard(board, size);
    } 

    while(1) {
        if (hasLegalMove(board, size, playerColour)) {

            if (COMPUTER_PLAYS_FOR_HUMAN) {
                findMove(board, size, playerColour, &bestRow, &bestCol, MAX_DEPTH);
                move[1] = bestRow + 'a';
                move[2] = bestCol + 'a';                
                printf("\nComputer places %c for Player at %c%c.\n", playerColour, move[1], move[2]);
            } else {
                printf("\nEnter move for colour %c (RowCol): ", playerColour);
                scanf("%s", input);
                move[1] = input[0];
                move[2] = input[1];
            }
            move[0] = playerColour;

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
            result = winner(board, size);
            if (result.winner == 'D') {
                printf("\nYou both suck\n\n");
            } else {
                printf("\n%c: %d, %c: %d --- %c player wins.\n\n",
                BLACK, result.blackScore, WHITE, result.whiteScore, result.winner);
            }
            return 0;
        }
        move[0] = computerColour;
        if (hasLegalMove(board, size, computerColour)) {

            getrusage(RUSAGE_SELF, &usage);
            start = usage.ru_utime;
            timeStart = start.tv_sec + start.tv_usec / 1000000.0; // in seconds

            findMove(board, size, computerColour, &bestRow, &bestCol, MAX_DEPTH, MAX_TIME);
            move[1] = bestRow + 'a';
            move[2] = bestCol + 'a';
            flipPieces(board, size, move);
            printf("\nComputer places %c at %c%c.\n", computerColour, move[1], move[2]);
            totalMovesCounter++;
            printBoard(board, size);
        } else {
            printf("Computer player %c has no valid move.\n", computerColour);
        }
        if (!hasLegalMove(board, size, computerColour) && !hasLegalMove(board, size, playerColour)) {
            result = winner(board, size);
            if (result.winner == 'D') {
                printf("You both suck");
            } else {
                printf("\n%c: %d, %c: %d --- %c player wins.\n\n",
                BLACK, result.blackScore, WHITE, result.whiteScore, result.winner);
            }
            return 0;
        }
    }
    return 0;
}