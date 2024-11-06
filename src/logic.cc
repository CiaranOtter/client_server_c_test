// tic_tac_toe_logic.cpp
#include "logic.h"

TicTacToeLogic::TicTacToeLogic() 
    : board_(9, ' '), current_player_('X') {}

bool TicTacToeLogic::validateAction(const messages::Action& action) {
    int position = action.x();
    return position >= 0 && position < 9 && board_[position] == ' ';
}

bool TicTacToeLogic::processAction(const messages::Action& action) {
    int position = action.x();
    board_[position] = current_player_;
    current_player_ = (current_player_ == 'X') ? 'O' : 'X';
    return true;
}

messages::Match TicTacToeLogic::getMatchState() {
    messages::Match match;
    // Fill in match state from board_
    return match;
}

messages::Result TicTacToeLogic::getFinalResults() {
    messages::Result result;
    // Fill in final results based on game outcome
    return result;
}

bool TicTacToeLogic::checkWin() const {
    // Implement win checking logic
    return false;
}

bool TicTacToeLogic::checkDraw() const {
    return std::count(board_.begin(), board_.end(), ' ') == 0;
}