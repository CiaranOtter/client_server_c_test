// referee_server.cpp
#include "referee_server.h"
#include <thread>

RefereeServer::RefereeServer(const std::string& stream_service_address) 
    : stream_service_address_(stream_service_address) {}

void RefereeServer::RegisterGameLogic(std::unique_ptr<GameLogicInterface> game_logic) {
    game_logic_ = std::move(game_logic);
}

void RefereeServer::ConnectToStreamService() {

    messages::Resp response;
    stream_service_channel_ = grpc::CreateChannel(
        stream_service_address_, grpc::InsecureChannelCredentials());

    stream_service_stub_ = stream::StreamService::NewStub(stream_service_channel_);
    
    stream_context_ = std::make_unique<grpc::ClientContext>();
    stream_writer_ = stream_service_stub_->SendStream(stream_context_.get(), &response);
}

void RefereeServer::QueueStreamMessage(messages::MatchPlayMessages&& message) {

    messages::MatchPlayMessages match_message;
    match_message.CopyFrom(message); // Copy `message` correctly

    messages::StreamMessages stream_message;
    stream_message.set_allocated_streammessages(new messages::MatchPlayMessages(std::move(match_message)));
    

    {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        message_queue_.push(std::move(stream_message));
    }
    queue_cv_.notify_one();
}

messages::StreamMessages RefereeServer::WaitForNextMessage() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_cv_.wait(lock, [this] { 
        return !message_queue_.empty() || !is_running_; 
    });
    
    if (!is_running_) {
        return messages::StreamMessages{};
    }
    
    messages::StreamMessages message = std::move(message_queue_.front());
    message_queue_.pop();
    return message;
}

void RefereeServer::StartStreamingSender() {
    while (is_running_) {
        messages::StreamMessages message = WaitForNextMessage();
        if (!is_running_) break;
        
        if (!SendStreamMessage(message)) {
            // Handle streaming error
            break;
        }
    }
}

bool RefereeServer::SendStreamMessage(const messages::StreamMessages& message) {
    messages::Resp response;
    return stream_writer_->Write(message);
}

void RefereeServer::Run(const std::string& server_address) {
    GameServiceImpl service(this);
    
    grpc::ServerBuilder builder;
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    builder.RegisterService(&service);
    
    server_ = builder.BuildAndStart();
    is_running_ = true;
    
    // Connect to stream service and start streaming thread
    ConnectToStreamService();
    std::thread stream_thread(&RefereeServer::StartStreamingSender, this);
    
    server_->Wait();
    is_running_ = false;
    queue_cv_.notify_all();
    stream_thread.join();
}

void RefereeServer::Stop() {
    if (server_) {
        is_running_ = false;
        queue_cv_.notify_all();
        server_->Shutdown();
    }
}

RefereeServer::GameServiceImpl::GameServiceImpl(RefereeServer* server) 
    : referee_server_(server) {

        player_count = 0;
        move_available = false;
    }

grpc::Status RefereeServer::GameServiceImpl::PlayGame(
    grpc::ServerContext* context,
    grpc::ServerReaderWriter<messages::MatchPlayMessages,messages::MatchPlayMessages>* stream) {
    
    // Handle bidirectional streaming

    constants::Command cur_command = constants::Command::GENERATE_MOVE;

    messages::MatchPlayMessages request;
    while (stream->Read(&request)) {

        std::cout << "Received a message from a client" << std::endl;
        messages::MatchPlayMessages response;

        referee_server_->QueueStreamMessage(std::move(request));

        switch (request.command()) {
            case constants::Command::CONNECT: {
                std::cout << "A new player has connected" << std::endl;

                messages::Player player = request.player();

                std::cout << player.playername() << " has joined the game" << std::endl;
                player_count += 1;
                game_ready.notify_one();
                std::unique_lock<std::mutex> lock(game_ready_m);
                game_ready.wait(lock, [this] {
                    return player_count == 2;
                });
                
                if (!stream->Write(request)) {
                    return grpc::Status(grpc::StatusCode::ABORTED, "Failed to send response to player to start the game");
                }
                break;
            }
            case constants::Command::PLAY_MOVE: {
                std::cout << "The player has played a move" << std::endl;
                move_available = true;
                move_available_c.notify_one();
                break;
            }
            case constants::Command::GET_COMMAND: {
                std::cout << "The player has requested a command" << std::endl; 

                messages::MatchPlayMessages command;
                command.set_command(cur_command);

                std::cout << "Seding the player a command" << std::endl;
                

                if (cur_command == constants::Command::GENERATE_MOVE) {
                    cur_command = constants::Command::PLAY_MOVE;
                } else {
                    {
                        std::mutex await_move;
                        std::unique_lock<std::mutex> move_lock(await_move);
                        move_available_c.wait(move_lock, [this] {
                            return move_available;
                        });
                        cur_command = constants::Command::GENERATE_MOVE;
                        command.mutable_action()->set_x(1);
                        move_available = false;
                    }
                    
                }

                if (!stream->Write(command)) {
                    return grpc::Status(grpc::StatusCode::ABORTED, "Failed to send a command to the player");
                }
                std::cout << "The player has been sent a command" << std::endl;

                break;
            }
            default: {
                std::cout << "Something went wrong" << std::endl;
                break;
            }
        }

        // if (request.command() == constants::Command::CONNECT) {
        //     std::cout << "A new palyer has connected" << std::endl;

        // // } else if (request.has_action()) {
        //     const auto& action = request.action();
            
        //     // Validate action
        //     if (!referee_server_->game_logic_->validateAction(action)) {
        //         response.set_command(constants::Command::RECV_FAIL);
        //         response.mutable_error()->set_errormessage("Invalid action");
        //         stream->Write(response);
        //         continue;
        //     }
            
        //     // Process action
        //     if (!referee_server_->game_logic_->processAction(action)) {
        //         response.set_command(constants::Command::RECV_FAIL);
        //         response.mutable_error()->set_errormessage("Action processing failed");
        //         stream->Write(response);
        //         continue;
        //     }
            
        //     // Create stream message for the stream service
        //     messages::StreamMessages stream_msg;
        //     auto* play_msg = stream_msg.mutable_streammessages();
        //     play_msg->CopyFrom(request);
        //     referee_server_->QueueStreamMessage(std::move(stream_msg));
            
        //     // Send success response to client
        //     response.set_command(constants::Command::PLAY_MOVE);
        //     stream->Write(response);

        //     // Check if game is complete
        //     // if (referee_server_->game_logic_->getFinalResults()._winner()) {
        //     //     messages::StreamMessages result_msg;
        //     //     result_msg.mutable_results()->CopyFrom(
        //     //         referee_server_->game_logic_->getFinalResults());
        //     //     referee_server_->QueueStreamMessage(std::move(result_msg));
        //     //     break;
        //     // }
        // }
    }

    return grpc::Status::OK;
}
// #include<iostream>
// #include <iostream>
// #include <memory>
// #include <string>
// #include <thread>
// #include <chrono>

// #include <grpcpp/grpcpp.h>
// #include <grpcpp/ext/proto_server_reflection_plugin.h>
// #include<grpcpp/grpcpp.h>
// #include "stream.grpc.pb.h"
// #include "messages.pb.h"
// #include "match_play.pb.h"
// #include "match_play.grpc.pb.h"
// #include "constants.pb.h"

// // #include"tournament_engine_API/game_comm/game_comm.grpc.pb.h"
// // /
// // #include"proto/

// using grpc::Channel;            // The channel for a gRPC connection
// using grpc::ClientContext;      // The client context for gRPC connections
// using grpc::Status;             // The Status of a gRPC message. This is used to signal errors
// using grpc::Server;             // The server for a gRPC server
// using grpc::ServerBuilder;      // The server builder for gRPC servers.
// using grpc::ServerContext;      // The server context for server side contexts in gRPC connections
// using grpc::ServerReaderWriter; // The reader writer for gRPC connections for bidirectional streaming
// using grpc::ClientWriter;       // The client side writer for gRPC client side streaming

// using stream::StreamService;            // The streaming service library.
// using messages::MatchPlayMessages;    // The match messages between players and referees.
// using match_play::MatchPlayService;     // The Service for streaming between players and referees

// using messages::StreamMessages;         // The streaming messages
// using messages::Match;                  // The match messages for creating new matches on the server side
// using messages::Error;                  // The Error for errors during gameplay
// using messages::Result;                 // The result message to return the results of a match
// using messages::Resp;                   // The base response message.
// using messages::Player;                 // The player message for connecting players to the match


// class GameServiceImpl final : public MatchPlayService::Service {


// public:
//     struct GameSession {
//         std::mutex mutex;
//         GameState current_state;
//         std::vector<ServerReaderWriter<GameState, GameMove>*> player_streams;
//     };

//     GameServiceImpl() {
//         // Initialize logging service connection
//         auto channel = grpc::CreateChannel(
//             "localhost:50052", grpc::InsecureChannelCredentials());
//         logging_stub_ = LoggingService::NewStub(channel);
//     }

//     Status PlayGame(ServerContext* context,
//                    ServerReaderWriter<GameState, GameMove>* stream) override {
//         GameMove move;
//         std::string game_id;
//         std::shared_ptr<GameSession> session;

//         // Read first move to get player information
//         if (!stream->Read(&move)) {
//             return Status(grpc::INVALID_ARGUMENT, "No initial move received");
//         }

//         // Create or join game session
//         {
//             std::lock_guard<std::mutex> lock(sessions_mutex_);
//             if (game_sessions_.empty() || 
//                 game_sessions_.begin()->second->player_streams.size() < 2) {
//                 // Create new game if needed
//                 if (game_sessions_.empty()) {
//                     game_id = GenerateGameId();
//                     session = std::make_shared<GameSession>();
//                     game_sessions_[game_id] = session;
//                 } else {
//                     // Join existing game
//                     auto it = game_sessions_.begin();
//                     game_id = it->first;
//                     session = it->second;
//                 }
//             } else {
//                 return Status(grpc::RESOURCE_EXHAUSTED, "No available games");
//             }
//         }

//         // Add player to session
//         {
//             std::lock_guard<std::mutex> lock(session->mutex);
//             session->player_streams.push_back(stream);
            
//             // Initialize player in game state
//             auto& player_state = (*session->current_state.mutable_players())[move.player_id()];
//             player_state.set_player_id(move.player_id());
//             player_state.set_score(0);
//             player_state.set_status("active");
//         }

//         // Main game loop
//         while (true) {
//             if (!stream->Read(&move)) {
//                 // Player disconnected
//                 RemovePlayer(game_id, move.player_id());
//                 break;
//             }

//             // Process move and update game state
//             {
//                 std::lock_guard<std::mutex> lock(session->mutex);
//                 ProcessMove(session->current_state, move);
                
//                 // Broadcast updated state to all players
//                 for (auto player_stream : session->player_streams) {
//                     player_stream->Write(session->current_state);
//                 }

//                 // Log game event
//                 LogGameEvent(move, game_id);
//             }
//         }

//         return Status::OK;
//     }

//     Status LogGameEvents(ServerContext* context,
//                         ServerReader<GameEvent>* reader,
//                         LogResponse* response) override {
//         GameEvent event;
//         while (reader->Read(&event)) {
//             // Forward event to logging service
//             ClientContext logging_context;
//             LogResponse log_response;
            
//             Status status = logging_stub_->LogEvent(&logging_context, event, &log_response);
            
//             if (!status.ok()) {
//                 response->set_success(false);
//                 response->set_message("Failed to log event: " + status.error_message());
//                 return status;
//             }
//         }
        
//         response->set_success(true);
//         response->set_message("Events logged successfully");
//         return Status::OK;
//     }

// private:
//     std::mutex sessions_mutex_;
//     std::unordered_map<std::string, std::shared_ptr<GameSession>> game_sessions_;
    
//     // Client for logging server
//     std::unique_ptr<LoggingService::Stub> server_stub_;

//     std::string GenerateGameId() {
//         static int counter = 0;
//         return "game_" + std::to_string(++counter);
//     }

//     void ProcessMove(GameState& state, const GameMove& move) {
//         // Update game state based on move
//         // This is where you'd implement your game logic
        
//         // Example: Update player's score based on move
//         auto& player = (*state.mutable_players())[move.player_id()];
//         player.set_score(player.score() + 1);
        
//         // Check for game over conditions
//         // ...
//     }

//     void RemovePlayer(const std::string& game_id, const std::string& player_id) {
//         std::lock_guard<std::mutex> lock(sessions_mutex_);
//         if (auto it = game_sessions_.find(game_id); it != game_sessions_.end()) {
//             auto session = it->second;
//             std::lock_guard<std::mutex> session_lock(session->mutex);
            
//             // Update player status
//             auto& player = (*session->current_state.mutable_players())[player_id];
//             player.set_status("disconnected");
            
//             // Remove game session if all players disconnected
//             bool all_disconnected = true;
//             for (const auto& player_pair : session->current_state.players()) {
//                 if (player_pair.second.status() == "active") {
//                     all_disconnected = false;
//                     break;
//                 }
//             }
            
//             if (all_disconnected) {
//                 game_sessions_.erase(it);
//             }
//         }
//     }

//     void LogGameEvent(const GameMove& move, const std::string& game_id) {
//         GameEvent event;
//         event.set_game_id(game_id);
//         event.set_player_id(move.player_id());
//         event.set_event_type("move");
//         event.set_event_data("x:" + std::to_string(move.x()) + 
//                            ",y:" + std::to_string(move.y()) +
//                            ",type:" + move.move_type());
//         event.set_timestamp(move.timestamp());

//         ClientContext context;
//         LogResponse response;
//         logging_stub_->LogEvent(&context, event, &response);
//     }
// };

// int main(int argc, char** argv) {
//     std::string server_address("0.0.0.0:50051");
//     GameServiceImpl service;

//     ServerBuilder builder;
//     builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
//     builder.RegisterService(&service);

//     std::unique_ptr<Server> server(builder.BuildAndStart());
//     std::cout << "Game server listening on " << server_address << std::endl;
//     server->Wait();

//     return 0;
// }

// // class StreamClient {
// // public:
// //     StreamClient(std::shared_ptr<Channel> channel) : stub_(StreamService::NewStub(channel)) {}

// //     void StreamData() {
// //         // Data to be sent to the server
// //         messages::StreamMessages request;
// //         messages::Resp response;
// //         grpc::ClientContext context;

// //         writer = stub_->SendStream(&context, &response);

// //         // Create a stream for sending requests

// //         // // Send multiple requests to the server
// //         // for (int i = 0; i < 5; i++) {
// //         //     // request.set_data("Message " + std::to_string(i));
// //         //     request.set_allocated_match(new Match());
// //         //     if (!writer->Write(request)) {
// //         //         // Broken stream
// //         //         break;
// //         //     }
// //         //     std::this_thread::sleep_for(std::chrono::seconds(1)); // Simulate delay between messages
// //         // }

// //         // // Indicate that we're done sending messages
// //         // writer->WritesDone();

// //         // // Wait for the server to respond
// //         // Status status = writer->Finish(&response, &context);
// //         // if (status.ok()) {
// //         //     std::cout << "Received response: " << response.success() << std::endl;
// //         // } else {
// //         //     std::cerr << "RPC failed: " << status.error_message() << std::endl;
// //         // }
// //     }

// //     void connect_players() {

// //     }

// // private:
// //     std::unique_ptr<stream::StreamService::Stub> stub_;
// //     std::unique_ptr<grpc::ClientWriter<messages::StreamMessages>> writer;
// //     // gamestream;
// // };

// // // The match server for referee to player game play
// // class MatchPlayServiceImpl final : public MatchPlayService::Service  {
// //     public:

// //     StreamClient* client;
// //     // The service server streaming connection
// //     std::unique_ptr<grpc::ClientWriter<messages::StreamMessages>> stream;
// //     // std::unique_ptr<grpc::ClientWriter<messages::StreamMessages>> 

// //     void setStreamer() {
// //         client = new StreamClient(grpc::CreateChannel("localhost:5000", grpc::InsecureChannelCredentials()));        
// //     }

// //     int player_count = 0;

// //     Status HandleConnect(messages::MatchPlayMessages player) {
// //         if (!player.has_player()) {
// //             return Status(grpc::StatusCode::NOT_FOUND, "No player has been given");
// //         }

// //         messages::Player p = player.player();
// //         std::cout << "Player: " << p.playername() << " has connected" << std::endl; 


// //         messages::StreamMessages messages;
// //         messages.set_allocated_streammessages(&player);
// //         // messages::MatchPlayMessages* action = messages->mutable_streammessages();
// //         // action = &player;
// //         // writer->Write(messages::St)/

// //         return Status::OK;         
// //     }

// //     Status HandlePlayMove(messages::MatchPlayMessages move) {
// //         if (!move.has_action()) {
// //             return Status(grpc::StatusCode::NOT_FOUND,"No move action has been supplied");
// //         }

// //         messages::Action action = move.action();
// //         messages::Player player = move.player();

// //         std::cout << player.playername() << " has played a move" << std::endl;

// //         return Status::OK;
// //     }

// //     Status PlayGame(ServerContext* context,
// //                     ServerReaderWriter<MatchPlayMessages, MatchPlayMessages>* stream) override {
// //         MatchPlayMessages request;

// //         std::cout << "New player connected" << std::endl;

// //         bool player = false;
// //         while (stream->Read(&request)) {
// //             if (request.command() == constants::Command::CONNECT) {
// //                 Status s = HandleConnect(request);
// //                 if (s.error_code()!= grpc::OK) {
// //                     return s;
// //                 }
// //             } else if (request.command() == constants::Command::PLAY_MOVE) {
// //                 Status s = HandlePlayMove(request);
// //             }
// //         }
// //         return Status::OK;
// //     }
// // };


// // int main(int argc, char** argv) {
// //     // Create a channel to connect to the server

// //     std::string server_address("0.0.0.0:50051");
// //     MatchPlayServiceImpl match;
// //     match.setStreamer();


// //     ServerBuilder builder;
// //     builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
// //     builder.RegisterService(&match);

// //     std::unique_ptr<grpc::Server> gamestream(builder.BuildAndStart());
// //     std::cout << "Server listening on " << server_address << std::endl;

// //     gamestream->Wait();

    
    
// //     return 0;
// // }
// // // // Implements the services defined in the protobuf
// // class CommServiceImpl: public game_comm::GameComm::Service {

// //     private:
// //         char* ManangerAddress;
// //         bool Live;
// //         grpc::ClientContext* ctx;
// //         std::shared_ptr<grpc::ClientReaderWriter<game_comm::GameEvent, game_comm::GameEvent>> agentConn;
// //         std::unique_ptr<agent::AgentService::Stub> stub;
// //         std::shared_ptr<grpc::Channel> channel;

// //     public:
// //         CommServiceImpl() {
// //             this->Live = false;
// //         }

// //         CommServiceImpl(char* address) {

// //             channel = grpc::CreateChannel(address, grpc::InsecureChannelCredentials());
    
// //             // create a agent connection stub
// //             stub = agent::AgentService::NewStub(channel);
// //             std::cout << "Agent stub created" << std::endl;

// //             ctx = new grpc::ClientContext();

// //             agentConn = stub->ConnectToManager(ctx);
            
// //             std::cout << "Stream established" << std::endl;

// //             this->Live = true;
// //         }

// //     CommServiceImpl::~CommServiceImpl() {
// //         if (this->Live) {
// //             // Close the connection
// //         }
// //     }

// //     int num_players = 0;
// //     bool awaiting_move = true;
// //     int last_move;
// //     int player_turn = 0;
// //     // ::grpc::ServerReaderWriter< ::Move, ::Move>*[2] players;


// //     ::grpc::Status GameStream(::grpc::ServerContext* context, ::grpc::ServerReaderWriter< game_comm::Move, game_comm::Move>* stream) override{

// //         std::cout << "New client stream created" << std::endl;

// //         game_comm::Move move;

// //         bool player_connected = false;
// //         int player_index = 0;
// //         bool My_turn = false;

// //         bool running = true;
// //         int total_messages = 0;

// //         std::string player_name = "";

// //         while (stream->Read(&move)) {

// //             std::cout << "Received message" << std::endl;

// //             switch(move.command()) {
// //                 // if the Referee recieves a message for player connection request
// //                 case game_comm::Command::CONNECT:
// //                     std::cout << "connection request" << std::endl;
// //                     if (!move.has_player()) {
// //                         return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "No given player");
// //                     }
// //                     player_connected = true;
// //                     player_name = move.player();

// //                     player_index = num_players;
// //                     num_players++;

// //                     std::cout << "New player connected " << player_name << " " << player_index << std::endl;

// //                     player_turn = 0;

// //                     // await for other players to successfully connect
// //                     while(num_players < 2);

// //                     // Set the connection message to true
// //                     move.set_success(true);

// //                     // Write the success connect message back to the client
// //                     if (!stream->Write(move)) {
// //                         std::cout << "There was an error during connection" << std::endl;
// //                     } else {
// //                         std::cout << "Starting game" << std::endl;
// //                     }

// //                     break;
// //                 case game_comm::Command::GET_COMMAND: //  server has received a get request command from the player
// //                     // if it is this players turn 

// //                     total_messages++;

// //                     if (total_messages >= 10) {
// //                         move.set_command(game_comm::Command::GAME_TERMINATION);

// //                         if (!stream->Write(move)) {
// //                             std::cout << "Failed to send the message" << std::endl;
// //                         } else {
// //                             std::cout << "Returned game end message to the user" << std::endl;
// //                         }
// //                     }

// //                     if (player_turn == player_index) {
// //                         move.set_command(game_comm::Command::GENERATE_MOVE);

// //                         if (!stream->Write(move)) {
// //                             std::cout << "Failed to write command to stream" << std::endl;
// //                         } else {
// //                             std::cout << "Generate move " << player_name << std::endl;
// //                         }
// //                     } else {

// //                         int waiting_index = player_turn;

// //                         while (waiting_index == player_turn);
// //                         // std::cout << "Done awaiting the move " <<std::endl;

// //                         move.set_command(game_comm::Command::PLAY_MOVE);
// //                         move.set_move(last_move);

// //                         std::cout << "Apply " << last_move << " " << player_name << std::endl;
// //                         stream->Write(move);
// //                         My_turn = true;
// //                     }
// //                     break;
// //                 case game_comm::Command::PLAY_MOVE: // if the player requests to play a move

// //                     // confirm player has been connected properly
// //                     if (!player_connected) {
// //                         return ::grpc::Status(::grpc::StatusCode::FAILED_PRECONDITION, "Player connection not initialised");
// //                     }

// //                     // validate the players move
// //                     if (!move.has_move()) {
// //                         return ::grpc::Status(::grpc::StatusCode::INVALID_ARGUMENT, "No move has been supplied");
// //                     }


// //                     std::cout << player_name << " playing  " << move.move() << std::endl;
                    
// //                     move.set_command(game_comm::Command::GET_COMMAND);

// //                     last_move = move.move();
// //                     awaiting_move = false;
// //                     player_turn = (player_turn+1)%2;

// //                     std::cout << "The next turn belongs to " << player_turn << std::endl;

// //                     // return a reply to the player
// //                     if (!stream->Write(move)) {
// //                         std::cout << "Something went wrong sending a reply to the player" << std::endl;
// //                     }

// //                     break;
// //                 default:    // if an unknow commmand has been sent the engine
// //                     std::cout << "Some other command" << std::endl;
// //                     break;
// //             }
// //         }

// //         // the client's intput stream has come to an end
// //         std::cout << "Game has ended" << std::endl;

// //         // return message to confirm successfuly close of player stream
// //         return ::grpc::Status::OK;
// //     }
// // };

// // int startServer(int port) {
// //     // create a new instance of implemented service
// //     CommServiceImpl service;

// //     // create a grpc server builder
// //     grpc::ServerBuilder builder;

// //     char address[100];
// //     sprintf(address, "localhost:%d", port);
// //     // register server to address and port
// //     builder.AddListeningPort(address, grpc::InsecureServerCredentials());
    
// //     // register to service to the server
// //     builder.RegisterService(&service);

// //     // build and start the server
// //     std::unique_ptr<grpc::Server> server(builder.BuildAndStart());
// //     std::cout << "Server listening on " << "4000" << std::endl;

// //     // await for the end of the server
// //     server->Wait();

// //     return 0;
// // }

// // int StartLive(int local_port, char *address) {
// //     std::cout << "Attempting to connect to the manager" << std::endl;

    
// //     return 0;
// // }

// // void connectToMananger(char* address) {

// //     grpc::CreateChannel(address, grpc::InsecureChannelCredentials())


// //     Request request;
// //     Response response;
// //     ClientContext context;

// //     // Create a stream for sending requests
// //     std::unique_ptr<grpc::ClientWriter<Request>> writer(stub_->StreamData(&context));

// //     // create a channel to connect to server
    
// // }

// // // clas
// // int main(int argc, char* argv[]) {

// //     if (argc == 2) {
// //         std::cout << "Warning this server will be started in local mode" << std::endl;

// //         int port = atoi(argv[1]);
// //         return startServer(port);
// //         // start the local server without connection to the manager
// //     } else if (argc == 3) {
// //         // start the local server with connection to the manager
// //         int port = atoi(argv[1]);
// //         StartLive(port, argv[2]);
// //     } else {
// //         std::cout << "Usage: " << std::endl;
// //         std::cout << "For local usage: " << argv[0] << " <local referee server port>" << std::endl;
// //         std::cout << "For use with connection to external manager: " << argv[0] << " <local referee server port> <manager address>" << std::endl; 
// //         return 1;
// //     }

// //     // startServer();

// //     return 0;
// // }