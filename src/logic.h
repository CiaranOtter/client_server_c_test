#pragma once
#include "referee_server.h"

class TicTacToeLogic : public GameLogicInterface {
public:
    TicTacToeLogic();
    bool validateAction(const messages::Action& action) override;
    bool processAction(const messages::Action& action) override;
    messages::Match getMatchState() override;
    messages::Result getFinalResults() override;

private:
    std::vector<char> board_;
    char current_player_;
    bool checkWin() const;
    bool checkDraw() const;
};