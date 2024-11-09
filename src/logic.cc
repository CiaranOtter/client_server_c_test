#include <memory>
#include <stdexcept>
#include <cstring>
#include "logic.h"


GoGameLogic::GoGameLogic() {
    // Initialize the game board

    opp_init(15);
}

GoGameLogic::~GoGameLogic() {
    delete[] legal_moves_;
    opp_free_board();
}

bool GoGameLogic::isMoveLegal(int position) {

    if (full()) {
        std::cout << "The board is full" << std::endl;
        return false;
    }
    num_legal_moves_ = 0;
    // Get current legal moves
    opp_legal_moves(legal_moves_, &num_legal_moves_);

    std::cout << "There are still " << num_legal_moves_ << " legal moves in the match" << std::endl;
    
    // Check if the position is in the legal moves array
    for (int i = 0; i < num_legal_moves_; i++) {
        std::cout << legal_moves_[i] << std::endl;
        if (legal_moves_[i] == position) {
            return true;
        }
    }

    std::cout << "failed to find the move in the list of legal moves" << std::endl;
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
    std::cout << "Processing the action" << std::endl;
    try {
        int position = action.x();
        if (!isMoveLegal(position)) {
            return false;
        }
        
        // Make the move
        opp_make_move(position, 1);
        
        // Update match state
        // updateMatchState();
        
        std::cout << "The action has been processed" << std::endl;
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