#include<grpcpp/grpcpp.h>
#include <grpcpp/channel.h>
#include <grpcpp/client_context.h>
#include <grpcpp/create_channel.h>
#include"proto/game_comm.grpc.pb.h"
#include"proto/game_comm.pb.h"
#include "comm_client.h"
#include<cstdlib>
#include<string>



std::shared_ptr<grpc::ClientReaderWriter<Move, Move>> referee;
grpc::CompletionQueue *cq;
std::shared_ptr<grpc::Channel> channel;
std::unique_ptr<GameComm::Stub> stub;
grpc::ClientContext *context;


int test() {
    ::Move connect;
    connect.set_command(Command::CONNECT);
    connect.set_player("Ciaran");
    std::cout << "Writing messsage" <<std::endl;

    referee->Write(connect);
    referee->Read(&connect);

    std::cout << "This was a success" << std::endl;

    return 0;
}

int initialise_comms(char* address, int port) {

    char* full_addr = (char*)malloc(sizeof(address)+sizeof(port)+sizeof(char));
    sprintf(full_addr, "%s:%d", address, port);

    std::cout << "Starting connection" <<std::endl;
    channel = grpc::CreateChannel(full_addr, grpc::InsecureChannelCredentials());
    std::cout << "Channel created" << std::endl;

    stub = GameComm::NewStub(channel);

    // test = &(new Stream());

    context = new grpc::ClientContext();

    std::cout << "Stub created" << std::endl;

    referee = stub->GameStream(context);
    // referee.reset(temp);
    // referee = s;

    std::cout << "Stream established" << std::endl;
    test();

    // int t = 5;
    // receive_message(&t);

    // ::Move connect;
    // connect.set_command(Command::CONNECT);

    // connect.set_player("Ciaran");

    // // std::cout << referee <<std::endl;
    // referee->Write(connect);
    // referee->Read(&connect);

    // if (connect.success()) {
    //     std::cout << "Connected successfully" << std::endl;
    // } else {
    //     return 1;
    // }

    return 1;
}

int receive_message(int *move) {

    std::cout << "Running receive message" << std::endl;

    ::Move message;
    message.set_command(Command::GET_COMMAND);

    std::cout << "Message has been created" << std::endl;

    referee->Write(message);

    std::cout << "The message has been written to the referee" << std::endl;
    
    referee->Read(&message);
    std::cout << "I have received a message" << std::endl;

    *move = message.move();

    std::cout << "Command value is: " << message.command() << std::endl;
    return message.command();
}

int send_move(char *move) {

    std::cout << "The player wants to play " << move << std::endl;

    Move message;
    message.set_command(Command::PLAY_MOVE);
    message.set_move(*move);

    referee->Write(message);

    std::cout << "The move has been sent to be played" << std::endl;

    return 0;
}

void close_comms(void) {
    std::cout << referee.use_count() << std::endl;


    grpc::Status status;
    referee->WritesDone();
}


// int main(int argc, char** argv) {
//     std::cout << "HEllo client" << std::endl;

//     auto channel = grpc::CreateChannel("localhost:80", grpc::InsecureChannelCredentials());

//     auto stub = ::GameComm::NewStub(channel);

//     // connectPlayer(&stub);

//     grpc::ClientContext context;

//     std::shared_ptr<grpc::ClientReaderWriter<::Move, ::Move>> stream(stub->GameStream(&context));

//     ::Move connect;
//     connect.set_command(Command::CONNECT);

//     // send a connection message to the new stream

//     if (argc > 1) {
//         connect.set_player(argv[1]);
//     } else {
//         connect.set_player("Ciaran");

//     }
//     stream->Write(connect);

//     stream->Read(&connect); // this is the recee message function

//     if (!connect.success()) {
//         std::cout << "Failed to connect player" << std::endl;
//         return 1;
//     }

//     ::Move move;
//     move.set_command(Command::GET_COMMAND);
//     stream->Write(move); // this is the send message function

//     bool running = true;

//     // if (!status)
//     // template move sending

//     int cur_move = 0;
//     while (running) {
        
//         stream->Read(&move);

//         switch(move.command()) {
//             case Command::GENERATE_MOVE: // server requuests a move from the player
//                 std::cout << "Received a request to generate a move" << std::endl;

//                 move.set_command(Command::PLAY_MOVE);
//                 cur_move++;
//                 move.set_move(cur_move);
//                 stream->Write(move);
//                 break;
//             case Command::PLAY_MOVE: // server requests player to apply opponent move
//                 std::cout << "Received a request to apply an opponents move" << std::endl;

//                 std::cout << "Applying move " << move.move() << std::endl;

//                 move.set_command(Command::GET_COMMAND);
//                 stream->Write(move);

//                 break;

//             case Command::GAME_TERMINATION: // server has terminated the game
//                 std::cout << "The game has come to and end" << std::endl;
//                 running = false;
//                 break;
//             case Command::MATCH_RESET: // server has requested a match reset
//                 std::cout << "The match has been reset" << std::endl;

//                 running = false;
//                 break;
//             default:    
//                 std::cout << "Other command" << std::endl;
//                 move.set_command(Command::GET_COMMAND);
//                 stream->Write(move);
//                 break;
//         }

//     }

//     std::cout << "The sending action has ended" << std::endl;

//     stream->WritesDone(); // this is the close comms function

//     // std::cout << "writes done" << std::endl;
//     // stream->Finish();

//     std::cout << "Finish" << std::endl;

//     // requestMove(&stub);

//     return 0;
// }