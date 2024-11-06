#include "referee_server.h"
#include "logic.h"

int main(int argc, char** argv) {
    // Create the referee server with game server address
    RefereeServer referee("localhost:50051");
    
    // Create and register game logic
    auto game_logic = std::make_unique<TicTacToeLogic>();
    referee.RegisterGameLogic(std::move(game_logic));
    
    // Run the server

    std::cout << "Starting the game server" << std::endl;
    referee.Run("0.0.0.0:50052");
    
    return 0;
}