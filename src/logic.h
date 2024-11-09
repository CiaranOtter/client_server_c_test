
#ifdef __cplusplus

#include "referee_server.h"
#include <memory>
  // Only include this when compiling as C++

  // Forward declarations of the C functions
extern "C" {
#endif

#ifndef LOCAL_H
#define LOCAL_H

#define EMPTY -1
#define BLACK 0
#define WHITE 1

int opp_init(int);
void opp_run_master(int);

void opp_initialise_board(void);
void opp_free_board(void);
void opp_print_board(void);
void opp_reset_board(void);

int opp_random_strategy(int);
void opp_legal_moves(int*, int*);
void opp_make_move(int, int);

int get_winner(void);
bool full(void);
double* get_score(void);
bool game_over(void);

extern int BSIZE;
extern int *opp_board;
extern int opp_colour;
extern FILE *opp_fp;

#endif // LOCAL_H


#ifdef __cplusplus
}

class GoGameLogic : public GameLogicInterface {
public:
    GoGameLogic();
    ~GoGameLogic() override;
    
    bool validateAction(const messages::Action& action) override;
    bool processAction(const messages::Action& action) override;
    messages::Match getMatchState() override;
    messages::Result getFinalResults() override;

private:
    void updateMatchState();
    bool isMoveLegal(int position);
    
    int board_size_;
    int* legal_moves_;
    int num_legal_moves_;
    messages::Match current_state_;
    double* scores_;
    int* board;
};

#endif

