#include<iostream>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include<grpcpp/grpcpp.h>
#include"proto/game_comm.grpc.pb.h"
#include"proto/game_comm.pb.h"


// Implements the services defined in the protobuf
class CommServiceImpl: public GameComm::Service {

    int num_players = 0;
    bool awaiting_move = true;
    int last_move;
    int player_turn = 0;
    // ::grpc::ServerReaderWriter< ::Move, ::Move>*[2] players;


    ::grpc::Status GameStream(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< ::Move, ::Move>* stream) override{

        std::cout << "New client stream created" << std::endl;

        ::Move move;

        bool player_connected = false;
        int player_index = 0;
        bool My_turn = false;

        bool running = true;
        int total_messages = 0;

        std::string player_name = "";

        while (stream->Read(&move)) {

            switch(move.command()) {
                // if the Referee recieves a message for player connection request
                case Command::CONNECT:
                    std::cout << "connection request" << std::endl;
                    if (!move.has_player()) {
                        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "No given player");
                    }
                    player_connected = true;
                    player_name = move.player();

                    player_index = num_players;
                    num_players++;

                    std::cout << "New player connected " << player_name << " " << player_index << std::endl;

                    player_turn = 0;

                    // await for other players to successfully connect
                    while(num_players < 2);

                    // Set the connection message to true
                    move.set_success(true);

                    // Write the success connect message back to the client
                    if (!stream->Write(move)) {
                        std::cout << "There was an error during connection" << std::endl;
                    } else {
                        std::cout << "Starting game" << std::endl;
                    }

                    break;
                case Command::GET_COMMAND: //  server has received a get request command from the player
                    // if it is this players turn 

                    total_messages++;

                    if (total_messages >= 10) {
                        move.set_command(Command::GAME_TERMINATION);

                        if (!stream->Write(move)) {
                            std::cout << "Failed to send the message" << std::endl;
                        } else {
                            std::cout << "Returned game end message to the user" << std::endl;
                        }
                    }

                    if (player_turn == player_index) {
                        move.set_command(Command::GENERATE_MOVE);

                        if (!stream->Write(move)) {
                            std::cout << "Failed to write command to stream" << std::endl;
                        } else {
                            std::cout << "Generate move " << player_name << std::endl;
                        }
                    } else {

                        int waiting_index = player_turn;

                        while (waiting_index == player_turn);
                        // std::cout << "Done awaiting the move " <<std::endl;

                        move.set_command(Command::PLAY_MOVE);
                        move.set_move(last_move);

                        std::cout << "Apply " << last_move << " " << player_name << std::endl;
                        stream->Write(move);
                        My_turn = true;
                    }
                    break;
                case Command::PLAY_MOVE: // if the player requests to play a move

                    // confirm player has been connected properly
                    if (!player_connected) {
                        return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION, "Player connection not initialised");
                    }

                    // validate the players move
                    if (!move.has_move()) {
                        return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "No move has been supplied");
                    }


                    std::cout << player_name << " playing  " << move.move() << std::endl;
                    
                    move.set_command(Command::GET_COMMAND);

                    last_move = move.move();
                    awaiting_move = false;
                    player_turn = (player_turn+1)%2;

                    std::cout << "The next turn belongs to " << player_turn << std::endl;

                    // return a reply to the player
                    if (!stream->Write(move)) {
                        std::cout << "Something went wrong sending a reply to the player" << std::endl;
                    }

                    break;
                default:    // if an unknow commmand has been sent the engine
                    std::cout << "Some other command" << std::endl;
                    break;
            }
        }

        // the client's intput stream has come to an end
        std::cout << "Stream has ended" << std::endl;

        // return message to confirm successfuly close of player stream
        return ::grpc::Status::OK;
    }
};

void startServer() {
    // create a new instance of implemented service
    CommServiceImpl service;

    // create a grpc server builder
    grpc::ServerBuilder builder;

    // register server to address and port
    builder.AddListeningPort("localhost:4000", grpc::InsecureServerCredentials());
    
    // register to service to the server
    builder.RegisterService(&service);

    // build and start the server
    std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << "4000" << std::endl;

    // await for the end of the server
    server->Wait();

    
}
// clas
int main(int argc, char* argv[]) {

    startServer();

    return 0;
}