#include <memory>
#include <stdexcept>
#include <cstring>
#include "logic.h"


GoGameLogic::GoGameLogic() {
    // Initialize the game board

    BSIZE = 15;
    opp_initialise_board();
}

GoGameLogic::~GoGameLogic() {
    delete[] legal_moves_;
    opp_free_board();
}

bool GoGameLogic::isMoveLegal(int position) {
    // Get current legal moves
    opp_legal_moves(legal_moves_, &num_legal_moves_);
    
    // Check if the position is in the legal moves array
    for (int i = 0; i < num_legal_moves_; i++) {
        if (legal_moves_[i] == position) {
            return true;
        }
    }
    return false;
}

bool GoGameLogic::validateAction(const messages::Action& action) {
    try {
        int position = action.x();
        return isMoveLegal(position);
    } catch (const std::exception&) {
        return false;
    }
}

bool GoGameLogic::processAction(const messages::Action& action) {
    try {
        int position = action.x();
        if (!isMoveLegal(position)) {
            return false;
        }
        
        // Make the move
        opp_make_move(position, 1);
        
        // Update match state
        updateMatchState();
        
        return true;
    } catch (const std::exception&) {
        return false;
    }
}

void GoGameLogic::updateMatchState() {
    // // Update legal moves
    // opp_legal_moves(&legal_moves_, &num_legal_moves_);
    
    // // Get current scores
    // scores_ = get_score();
    
    // // Update the match state protocol buffer
    // current_state_.Clear();

    // for (int i = 0; i < num_legal_moves_; i++) {
    //     moves->add_positions(legal_moves_[i]);
    // }
    
    // // Add scores
    // auto* score = current_state_.mutable_score();
    // score->set_black(scores_[0]);
    // score->set_white(scores_[1]);
}

messages::Match GoGameLogic::getMatchState() {
    updateMatchState();
    return current_state_;
}

messages::Result GoGameLogic::getFinalResults() {
    messages::Result result;
    
    if (game_over()) {
        int winner = get_winner();
        if (winner > 0) {
            result.set_resulttype(constants::ResultEnum::WINNER);
            result.set_winner(winner);
        } else {
            result.set_resulttype(constants::ResultEnum::TIE);
            // result.set_winner("draw");
        }
        
        // Set final scores
        // auto* final_score = result.mutable_final_
        // final_score->set_black(scores_[0]);
        // final_score->set_white(scores_[1]);
    }
    
    return result;
}