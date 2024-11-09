// // #include<grpcpp/grpcpp.h>
// // #include <grpcpp/channel.h>
// // #include <grpcpp/client_context.h>
// // #include <grpcpp/create_channel.h>
// // #include "match_play.grpc.pb.h"
// // #include "messages.pb.h"
// // #include "match_play.pb.h"
// // #include "comm_client.h"
// // #include<cstdlib>
// // #include<string>
// // #include <filesystem>
// // // #include <mach-o/dyld.h>

// #include <grpcpp/grpcpp.h>
// #include "match_play.grpc.pb.h"
// #include <thread>


// class GameClient {
// public:
//     GameClient(const std::string& address) {
//         auto channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
//         stub_ = match_play::MatchPlayService::NewStub(channel);
//     }

//     void PlayGame() {
//         grpc::ClientContext context;
//         std::shared_ptr<grpc::ClientReaderWriter<messages::MatchPlayMessages, 
//                                                 messages::MatchPlayMessages>> stream(
//             stub_->PlayGame(&context));

//         // Start read thread
//         std::thread reader([stream]() {
//             messages::MatchPlayMessages response;
//             while (stream->Read(&response)) {
//                 // Handle server responses
//                 if (response.command() == constants::Command::RECV_FAIL) {
//                     std::cout << "Error: " << response.error().errormessage() << std::endl;
//                 }
//                 // Handle other responses...
//             }
//         });

//         // Send game actions
//         messages::MatchPlayMessages request;
//         request.set_command(constants::Command::GET_COMMAND);
//         auto* action = request.mutable_action();
//         // Set action properties...
        
//         stream->Write(request);

//         reader.join();
//         stream->WritesDone();
//     }

// private:
//     std::unique_ptr<match_play::MatchPlayService::Stub> stub_;
// };

// std::shared_ptr<grpc::ClientReaderWriter<match_play::MatchPlayMessages, match_play::MatchPlayMessages>> referee;
// grpc::CompletionQueue *cq;
// std::shared_ptr<grpc::Channel> channel;
// std::unique_ptr<match_play::MatchPlayService::Stub> stub;
// grpc::ClientContext *context;

// messages::Player player;

// int test() {
//     char path[PATH_MAX];
//     match_play::MatchPlayMessages connect;
//     connect.set_command(match_play::Command::CONNECT);
//     messages::Player* player = connect.mutable_player();

//     char result[PATH_MAX];
//     ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
//     std::string player_name;
//     if (count != -1) {
//         std::filesystem::path execPath(result, result + count);
//         player_name = execPath.filename().string();
//     } else {
//         player_name = "test";
//     }

//     std::cout << "the players naem is: " << player_name << std::endl;
//     player->set_playername(player_name);

//     std::cout << "Writing messsage" <<std::endl;

//     referee->Write(connect);
//     // exit(1);
//     referee->Read(&connect);

//     std::cout << "This was a success" << std::endl;

//     return 0;
// }

// int initialise_comms(char* address, int port) {

//     char* full_addr = (char*)malloc(sizeof(address)+sizeof(port)+sizeof(char));
//     sprintf(full_addr, "%s:%d", address, port);

//     std::cout << "Starting connection" <<std::endl;
//     channel = grpc::CreateChannel(full_addr, grpc::InsecureChannelCredentials());
//     std::cout << "Channel created" << std::endl;

//     stub = match_play::MatchPlayService::NewStub(channel);

//     // test = &(new Stream());

//     context = new grpc::ClientContext();

//     std::cout << "Stub created" << std::endl;

//     referee = stub->PlayGame(context);
//     // referee.reset(temp);
//     // referee = s;

//     std::cout << "Stream established" << std::endl;
//     test();

//     // int t = 5;
//     // receive_message(&t);

//     // ::MatchPlayMessages connect;
//     // connect.set_command(Command::CONNECT);

//     // connect.set_player("Ciaran");

//     // // std::cout << referee <<std::endl;
//     // referee->Write(connect);
//     // referee->Read(&connect);

//     // if (connect.success()) {
//     //     std::cout << "Connected successfully" << std::endl;
//     // } else {
//     //     return 1;
//     // }

//     return 1;
// }

// int receive_message(int *move) {

//     std::cout << "Running receive message" << std::endl;

//     match_play::MatchPlayMessages message;
//     message.set_command(match_play::Command::GET_COMMAND);

//     std::cout << "Message has been created" << std::endl;

//     referee->Write(message);

//     std::cout << "The message has been written to the referee" << std::endl;
    
//     referee->Read(&message);
//     std::cout << "I have received a message" << std::endl;

//     // if message.command() == stream
//     // *move = message.messag;

//     // std::cout << "Command value is: " << message.command() << std::endl;
//     return message.command();
// }

// int send_move(char *move) {

//     std::cout << "The player wants to play " << move << std::endl;

//     match_play::MatchPlayMessages message;
//     message.set_command(match_play::Command::PLAY_MOVE);
//     messages::Player* p = message.mutable_player();
//     p = &player;
    
//     match_play::Action* action = message.mutable_action();
//     action->set_x(*move);

//     referee->Write(message);

//     std::cout << "The move has been sent to be played" << std::endl;

//     return 0;
// }

// void close_comms(void) {
//     std::cout << referee.use_count() << std::endl;


//     grpc::Status status;
//     referee->WritesDone();
// }


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




#include "comm_client.h"
#include <memory>
#include <string>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <grpcpp/grpcpp.h>
#include "match_play.grpc.pb.h"

namespace {
    // Global state
    std::unique_ptr<match_play::MatchPlayService::Stub> stub_;
    std::shared_ptr<grpc::ClientReaderWriter<messages::MatchPlayMessages, 
                                            messages::MatchPlayMessages>> stream_;
    std::unique_ptr<grpc::ClientContext> context_;
    messages::Player player;
    
    // Message queue for received messages
    std::queue<int> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    bool is_running_ = false;
    std::thread reader_thread_;

    // Reader thread function
    void MessageReader() {
        messages::MatchPlayMessages response;
        while (is_running_ && stream_->Read(&response)) {
            int msg_type = 0;
            
            // Convert gRPC messages to C-friendly integers
            switch (response.command()) {
                case constants::Command::GET_COMMAND:
                    msg_type = 1;
                    break;
                case constants::Command::RECV_FAIL:
                    msg_type = 2;
                    break;
                case constants::Command::PLAY_MOVE:
                    msg_type = 3;
                    break;
                // case constants::Command::GAME_END:
                //     msg_type = 4;
                //     break;
                default:
                    msg_type = -1;
            }

            {
                std::lock_guard<std::mutex> lock(queue_mutex_);
                message_queue_.push(msg_type);
            }
            queue_cv_.notify_one();
        }
    }
}

const char* playername = "Player1"; // Set your player name here
// const messages::Player player;

int initialise_comms(char* address, int port) {
    try {
        // Create channel and stub
        std::string server_address = std::string(address) + ":" + std::to_string(port);
        auto channel = grpc::CreateChannel(server_address, grpc::InsecureChannelCredentials());
        std::cout << "Channel created" << std::endl;
        stub_ = match_play::MatchPlayService::NewStub(channel);
        std::cout << "Stub created" << std::endl;

        // Initialize streaming
        context_ = std::make_unique<grpc::ClientContext>();
        std::cout << "Connecting to stream" << std::endl;
        stream_ = stub_->PlayGame(context_.get());
        std::cout << "connected to stream" << std::endl;

        // Start reader thread
        is_running_ = true;
        // reader_thread_ = std::thread(MessageReader);
        // messages:Player player;
        // player.set_playername("player 1");

        messages::MatchPlayMessages connect_request;
        connect_request.set_command(constants::Command::CONNECT);
        messages::Player* temp = connect_request.mutable_player();
        temp->set_playername("Player1");

        if (!stream_->Write(connect_request)) {
            std::cout << "Failed to connect to the referee" << std::endl;
            return 0;
        }

        std::cout << "Awaiting game ready response" << std::endl;
        stream_->Read(&connect_request);
        std::cout << "Game is ready" << std::endl;
        return 1; // Success
    } catch (const std::exception&) {
        return 0; // Error
    }
}

int send_move(char* move) {
    try {

        std::cout << "Sending a move to the server: " << move << std::endl;
        messages::MatchPlayMessages request;
        request.set_command(constants::Command::PLAY_MOVE);
        auto* action = request.mutable_action();
        
        // Parse move string and set action properties
        // This will depend on your specific game's move format
        int x = atoi(move);
        action->set_x(x);
        
        if (!stream_->Write(request)) {
            return 0;
        }
        return 1; // Success
    } catch (const std::exception&) {
        return 0; // Error
    }
}

int receive_message(int* msg) {
    try {

        messages::MatchPlayMessages request; // send a request
        messages::MatchPlayMessages response; // holder for the repsonse

        // request the server for a command
        request.set_command(constants::Command::GET_COMMAND);

        if (!stream_->Write(request)) { // write the message
            return GAME_TERMINATION;
        }

        // Read the response from the server
        std::cout << "Requested the server for a command" << std::endl;
        stream_->Read(&response);
        std::cout << "Recieved a response from the server for move" << std::endl; 


        switch(response.command()) {
            case constants::Command::GENERATE_MOVE: // server requuests a move from the player
                std::cout << "Received a request to generate a move" << std::endl;
                return GENERATE_MOVE;
            case constants::Command::PLAY_MOVE: // server requests player to apply opponent move
                std::cout << "Received a request to apply an opponents move" << std::endl;
                return PLAY_MOVE;
            case constants::Command::GAME_TERMINATION: // server has terminated the game
                std::cout << "The game has come to and end" << std::endl;
                return GAME_TERMINATION;
            case constants::Command::MATCH_RESET: // server has requested a match reset
                std::cout << "The match has been reset" << std::endl;
                return MATCH_RESET;
            default:    
                std::cout << "Other command" << std::endl;
                return UNKNOWN;
        }
        // std::unique_lock<std::mutex> lock(queue_mutex_);
        // bool has_message = queue_cv_.wait_for(lock, 
        //     std::chrono::seconds(1), 
        //     []{return !message_queue_.empty();});

        // if (!has_message) {
        //     return -1; // Timeout
        // }

        // *msg = message_queue_.front();
        // message_queue_.pop();
        // return 0; // Success
    } catch (const std::exception&) {
        return constants::Command::GAME_TERMINATION; // Error
    }
}

void close_comms(void) {
    if (is_running_) {
        is_running_ = false;
        if (stream_) {
            stream_->WritesDone();
        }
        if (reader_thread_.joinable()) {
            reader_thread_.join();
        }
        context_.reset();
        stream_.reset();
        stub_.reset();
    }
}