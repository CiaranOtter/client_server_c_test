/********************************************************************
 *
 * This contains the code for the communication between my_player and the local opponent. 
 * Author: Joshua James Venter
 * Date: 2024/01/07
 *
 *
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "comms.h"
#include "local_opp.h"

const int PLAYER_TURN = 1;
const int OPP_TURN = 2;
const int GAME_OVER = 3;
static int status;

/* Instead of initialising communications with the Ingenious Framework referee, initialise the local opponent */
int initialise_comms(unsigned long int ip, int port) {

    printf("Playing with a local referee\n");

    // Read the player colour from the file created by run.sh
    FILE *fp = fopen("colour.txt", "r");
    int player_colour;
    int c = fscanf(fp, "%i", &player_colour);
    if (c != 1) fprintf(stderr, "Error reading colour from colour.txt\n");
    fclose(fp);

    // Set the status so that the black player starts
    int opp_colour = (player_colour == BLACK) ? WHITE : BLACK;
    status = (opp_colour == BLACK) ? PLAYER_TURN: OPP_TURN;

    opp_init(opp_colour, 15);

    return 1;
}

/* Return the appropriate message, based on the current status and/or the opp_gen_move's return value */
int receive_message(int *move) {

    if (status == PLAYER_TURN) {
        return GENERATE_MOVE;
    } else if (status == OPP_TURN) {
            int game_over = opp_gen_move(move);
            if (game_over) {
                status = GAME_OVER;
                return GAME_TERMINATION;
            } else {
                status = PLAYER_TURN;
                return PLAY_MOVE;
            }
    } else if (status == GAME_OVER) {
        return GAME_TERMINATION;
    } else {
        return GAME_TERMINATION;
    }

    return UNKNOWN;
}

/* Apply my_player's move to the local opponent */
int send_move(char *move) {
    bool game_over = opp_apply_move(move);
    if (game_over == true) {
        status = GAME_OVER;
    } else {
        status = OPP_TURN;
    }

 return 0;
}

void close_comms(void) {
}
