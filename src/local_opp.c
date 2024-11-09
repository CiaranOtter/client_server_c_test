/************************************************************************     
 *
 *    This is a skeleton to guide development of Gomuku engines that is intended 
 *    to be used with the Ingenious Framework.
 *
 *    The skeleton has a simple random strategy that can be used as a starting point.
 *    Currently the master thread (rank 0) runs a random strategy and handles the 
 *    communication with the referee, and the worker threads currently do nothing. 
 *    Some form of backtracking algorithm, minimax, negamax, alpha-beta pruning etc. 
 *    in parallel should be implemented. 
 * 
 *    Therfore, skeleton code provided can be modified and altered to implement different 
 *    strategies for the Gomuku game. However, the flow of communication with the referee, 
 *    relies on the Ingenious Framework and should not be changed.
 *
 *    Each engine is wrapped in a process which communicates with the referee, by
 *    sending and receiving messages via the server hosted by the Ingenious Framework.
 *
 *    The communication enumes are defined in comms.h and are as follows:
 *            - GENERATE_MOVE: Referee is asking for a move to be made.
 *            - PLAY_MOVE: Referee is forwarding the opponent's move. For this engine to update the
 *                                    board state.
 *         - MATCH_RESET: Referee is asking for the board to be reset. For another game.
 *         - GAME_TERMINATION: Referee is asking for the game to be terminated.
 *
 *    IMPORTANT NOTE FOR DEBBUGING:
 *            - Print statements to stdout will most likely not be visible when running the engine with the
 *                Ingenious Framework. Therefore, it is recommended to print to a log file instead. The pointer
 *                to the log file is passed to the opp_initialise_master function.
 *
 *    Author: Joshua James Venter
 *    Date: 2024/01/07
 *
 ************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
// #include <mpi.h>
#include <arpa/inet.h>
#include <time.h>
// #include "comms.h"
#include "logic.h"

#define MAX_MOVES 361
#define KSIZE 5
#define OFFSET 0

const int DRAW_RESULT = -1;
const int NON_TERMINAL_RESULT = -2;
const char *OPP_NAME_LOG = "opponent.log";

int BSIZE;
int *opp_board;
int opp_colour;
FILE *opp_fp;

// void opp_run_master(int);

// void opp_initialise_board(void);
// void opp_free_board(void);
// void opp_print_board(void);
// void opp_reset_board(void);

// int opp_random_strategy(int);
// void opp_legal_moves(int *, int *);
// void opp_make_move(int, int);

// int get_winner(void);
// bool full(void);
// double *get_score(void);
// bool game_over(void);


int opp_init(int boardsize) {

    /* Initialise board */
    BSIZE = boardsize;
    opp_initialise_board();
    /* Initialise colour */
    // opp_colour = colour; 
    // printf("Opponent colour: %d, %s \n", opp_colour, (opp_colour == BLACK)?"black":"white");

    /* Open file for logging */
    // opp_fp = fopen(OPP_NAME_LOG, "w");
    // if (opp_fp == NULL) {
    //     printf("Could not open log file\n");
    //     return 0;
    // }

    // fprintf(opp_fp, "Communication initialised \n");
    // fprintf(opp_fp, "Let the game begin...\n");
    // fprintf(opp_fp, "My name: %s\n", OPP_NAME_LOG);
    // fprintf(opp_fp, "My colour: %d, %s\n", opp_colour, (opp_colour == BLACK)?"black":"white");
    // fprintf(opp_fp, "Board size: %d\n", BSIZE);
    // fprintf(opp_fp, "-----------------------------------\n");
    // opp_print_board();

    // fflush(opp_fp);

    return 1;
}

/**
 * Make move
 *
 */
bool opp_gen_move(int *move) {

    *move = opp_random_strategy(opp_colour);
    opp_print_board();

    return game_over(); 
}

bool opp_apply_move(char *move, int colour) {
    printf("Applying a move to the board\n");
    int opp_move = atoi(move);
    opp_make_move(opp_move, colour);

    // fprintf(opp_fp, "\nPlacing %s player's piece: row %d, col %d\n", (opp_colour == BLACK)?"white":"black", opp_move/BSIZE, opp_move%BSIZE);
    // opp_print_board();
    // fflush(opp_fp);

    return game_over();
}

/**
 * Resets the board to the initial state.
 *
 * @param opp_fp pointer to the log file
*/
void opp_reset_board(void) {

    fprintf(opp_fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    fprintf(opp_fp, "~~~~~~~~~~~~~ NEW MATCH ~~~~~~~~~~~~\n");
    fprintf(opp_fp, "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
    opp_free_board();
    opp_initialise_board();

    fprintf(opp_fp, "New board state:\n");
}

/**
 * Runs a random strategy. Chooses a random legal move and applies it to the board, then 
 * returns the move in the form of an integer (0-361).
 * 
 * @param opp_colour colour of the player
 * @param opp_fp pointer to the log file
*/
int opp_random_strategy(int opp_colour) {
    int number_of_moves = 0;
    int *moves = malloc(sizeof(int) * MAX_MOVES);

    opp_legal_moves(moves, &number_of_moves);

    srand(time(NULL));
    int random_index = rand() % number_of_moves;
    int move = moves[random_index];
    opp_make_move(move, opp_colour);
    free(moves);

    fprintf(opp_fp, "\nPlacing my piece: row %d, col %d\n", move/BSIZE, move%BSIZE);
    fflush(opp_fp);

    return move;
}

/**
 * Applies the given move to the board.
 *
 * @param move move to apply
 * @param opp_colour colour of the player
*/
void opp_make_move(int move, int colour) {
    opp_board[move] = colour;
}

/**
 * Gets a list of legal moves for the current board, and stores them in the moves array followed by a -1.
 * Also stores the number of legal moves in the number_of_moves variable.
 * 
 * @param moves array to store the legal moves in
 * @param number_of_moves variable to store the number of legal moves in
*/
void opp_legal_moves(int *moves, int *number_of_moves) {
    int i, j, k = 0;

    for (i = 0; i < BSIZE; i++) {
        for (j = 0; j < BSIZE; j++) {
            if (opp_board[i * BSIZE + j] == EMPTY) {
                moves[k++] = i * BSIZE + j;
                (*number_of_moves)++;
            }
        }
    }

    moves[k] = -1;
}

/**
 * Initialises the board for the game.
 */
void opp_initialise_board(void) {
    opp_board = malloc(sizeof(int) * BSIZE * BSIZE);
    memset(opp_board, EMPTY, sizeof(int) * BSIZE * BSIZE);
}

/**
 * Prints the board to the given file with improved aesthetics.
 * 
 * @param opp_fp pointer to the file to print to
 */
void opp_print_board(void) {
    fprintf(opp_fp, "    ");

    for (int i = 0; i < BSIZE; i++) {
        if (i < 9) {
            fprintf(opp_fp, "%d  ", i + OFFSET);
        } else {
            fprintf(opp_fp, "%d ", i + OFFSET);
        }
    }
    fprintf(opp_fp, "\n");

    fprintf(opp_fp, "   +");
    for (int i = 0; i < BSIZE; i++) {
        fprintf(opp_fp, "--+");
    }
    fprintf(opp_fp, "\n");

    for (int i = 0; i < BSIZE; i++) {
        fprintf(opp_fp, "%2d |", i + OFFSET);
        for (int j = 0; j < BSIZE; j++) {
            char piece = '.';
            if (opp_board[i * BSIZE + j] == BLACK) {
                    piece = 'B';
            } else if (opp_board[i * BSIZE + j] == WHITE) {
                    piece = 'W';
            }
            fprintf(opp_fp, "%c  ", piece);
        }
        fprintf(opp_fp, "|");
        fprintf(opp_fp, "\n");
    }

    fprintf(opp_fp, "   +");
    for (int i = 0; i < BSIZE; i++) {
        fprintf(opp_fp, "--+");
    }
    fprintf(opp_fp, "\n");

    fflush(opp_fp);
}

/**
 * Frees the memory allocated for the board.
 */
void opp_free_board(void) {
    free(opp_board);
}

/* Searches for K-in-a-row
 * Note that K pieces in a row is found when the variable in_a_row is equal to KSIZE - 1, 
 * because in_a_row is only inc for the 2nd to 5th pieces in a row. 
 */
int get_winner(void) {
    // Rows
    for (int row = 0; row < BSIZE; row++) {
        int last = EMPTY;
        int in_a_row = 0;
        for (int col = 0; col < BSIZE; col++) {
            int curr = opp_board[row * BSIZE + col];

            if (curr != EMPTY && curr == last) {
                in_a_row++;
                if (in_a_row == KSIZE - 1) {
                    fprintf(opp_fp, "Winner: %s\nk-in-a-row ending at %d,%d\n", (curr==BLACK)?"black":"white", row, col); 
                    printf("Winner: %s\nk-in-a-row ending at %d,%d\n", (curr==BLACK)?"black":"white", row, col); 
                    return curr;
                }
            } else {
                in_a_row = 0;
            }
            last = curr;
        }
    }
 
    // Columns
    for (int col = 0; col < BSIZE; col++) {
        int last = EMPTY;
        int in_a_row = 0;
        for (int row = 0; row < BSIZE; row++) {
            int curr = opp_board[row * BSIZE + col];
    
            if (curr != EMPTY && curr == last) {
                in_a_row++;
                if (in_a_row == KSIZE - 1) {
                    fprintf(opp_fp, "Winner: %s\nk-in-a-column ending at %d,%d\n", (curr==BLACK)?"black":"white", row, col); 
                    printf("Winner: %s\nk-in-a-row ending at %d,%d\n", (curr==BLACK)?"black":"white", row, col); 
                    return curr;
                }
            } else {
                in_a_row = 0;
            }
            last = curr;
        }
    }
    
    // Diagonals (right)
    // For each field
    for (int row = 0; row < BSIZE; row++) {
        for (int col = 0; col < BSIZE; col++) {
            // Try to descend
            int last = EMPTY; 
            int in_a_row = 0;
            for (int r = row, c = col; r < BSIZE && c < BSIZE; r++, c++) {
                int curr = opp_board[r * BSIZE + c];
                if (curr != EMPTY && curr == last) {
                    in_a_row++;
                    if (in_a_row == KSIZE - 1) {
                        fprintf(opp_fp, "Winner: %s\nk-in-a-right-diagonal ending at %d,%d\n", (curr==BLACK)?"black":"white", r, c); 
                        printf("Winner: %s\nk-in-a-row ending at %d,%d\n", (curr==BLACK)?"black":"white", r, c); 
                        return curr;
                    }
                } else {
                    in_a_row = 0;
                }
                last = curr;
            }
        }
    }
 
    // Diagonals (left)
    // For each field
    for (int row = 0; row < BSIZE; row++) {
        for (int col = 0; col < BSIZE; col++) {
            // Try to descend
            int last = EMPTY;
            int in_a_row = 0;
            for (int r = row, c = col; r < BSIZE && r >= 0 && c < BSIZE
                    && c >= 0; r++, c--) {
                int curr = opp_board[r * BSIZE + c];
                if (curr != EMPTY && curr == last) {
                    in_a_row++;
                    if (in_a_row == KSIZE - 1) {
                        fprintf(opp_fp, "Winner: %s\nk-in-a-left-diagonal ending at %d,%d\n", (curr==BLACK)?"black":"white", r, c);
                        printf("Winner: %s\nk-in-a-row ending at %d,%d\n", (curr==BLACK)?"black":"white", r, c);
                        return curr;
                    }
                } else {
                    in_a_row = 0;
                }
                last = curr;
            }
        }
    }

    if (full()) {
     fprintf(opp_fp, "Full board\n");
     return DRAW_RESULT; // draw
    }

    return NON_TERMINAL_RESULT; // not yet terminal
}

/* If the board has at least one empty slot, return false, otherwise return true */
bool full(void) {
    for (int i = 0; i < BSIZE; i++) {
        for (int j = 0; j < BSIZE; j++) {
            if (opp_board[i * BSIZE + j] == EMPTY) {
                return false;
            }
        }
    }
    return true;
}

double *get_score(void) {
    double *scores = (double *) malloc(2*sizeof(double));
    int winner = get_winner();
    if(winner == DRAW_RESULT || winner == NON_TERMINAL_RESULT) {
        fprintf(opp_fp, "Draw\n");
        scores[0] = 0.5;
        scores[1] = 0.5;
    } else {
        fprintf(opp_fp, "Winner: %s\n", (winner==BLACK)?"black":"white"); 
        scores[winner] = 1;
    }
    return scores;
}

/* If the game is over, prints the final board to the opponent's log file and returns true, otherwise returns false */ 
bool game_over(void) {
    if (get_winner() == NON_TERMINAL_RESULT) return false; 
    return true;
}
