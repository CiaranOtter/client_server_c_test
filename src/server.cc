// #pragma once
#include "referee_server.h"
#include "logic.h"

int main(int argc, char** argv) {
    // Create the referee server with game server address
    std::cout << "Connecting to streaming server" << std::endl;
    RefereeServer referee("localhost:5000");
    std::cout << "Connected to the streaming server" << std::endl;
    
    // Create and register game logic
    std::cout << "Loading game logic" << std::endl;
    auto game_logic = std::make_unique<GoGameLogic>();
    std::cout << "Game logic created" << std::endl;
    referee.RegisterGameLogic(std::move(game_logic));
    std::cout << "Game logic loaded" << std::endl;
    
    // Run the server

    std::cout << "Starting the game server" << std::endl;
    referee.Run("0.0.0.0:50052");
    
    return 0;
}