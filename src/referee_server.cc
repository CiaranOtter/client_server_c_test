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
    int player_colour = 0;

    messages::MatchPlayMessages request;
    while (stream->Read(&request)) {

        std::cout << "Received a message from a client" << std::endl;
        messages::MatchPlayMessages response;

        referee_server_->QueueStreamMessage(std::move(request));

        switch (request.command()) {
            case constants::Command::CONNECT: {
                std::cout << "A new player has connected" << std::endl;

                messages::Player player = request.player(); // Get the player from the request

                std::cout << player.playername() << " has joined the game" << std::endl;    

                player_count += 1;                          // Increase the total number of players connected to the game
                player_colour = request.player_postion();  // set the the colour of the player for the match

                if (request.player_postion() == 1) { 
                    // if the player is the first player set their first command to generate a move
                    cur_command = constants::Command::GENERATE_MOVE;
                } else {
                    // else prepare the player for an opponents move
                    cur_command = constants::Command::PLAY_MOVE;
                }

    
                game_ready.notify_one(); // if there are any players waiting for the game to start. Notify the new player to check if the game is ready
               
                std::unique_lock<std::mutex> lock(game_ready_m); // create a lock for the player to start waiting for the match to be ready
                game_ready.wait(lock, [this] {
                    return player_count == 2; // if the match has sufficient players, unlock the player
                });
                
                // send the ready match response to the player
                if (!stream->Write(request)) {
                    return grpc::Status(grpc::StatusCode::ABORTED, "Failed to send response to player to start the game");
                }
                break;
            }

            
            case constants::Command::PLAY_MOVE: {

                std::cout << "The player has played a move: " << request.action().x() << std::endl;

                move_available = true; // log that move is available
                last_move = request.action().x();

                if (!referee_server_->game_logic_->processAction(request.action())) {   // process the new move
                    referee_server_->is_running_ = false;   // if the move is illegal, end the match
                    std::cout << "An illegal move has been played" << std::endl;   
                    cur_command = constants::Command::GAME_TERMINATION;

                }
                
                move_available_c.notify_one();
                break;
            }
            case constants::Command::GET_COMMAND: {
                std::cout << "The player has requested a command" << std::endl; 

                

                std::cout << "Seding the player a command" << std::endl;
                
                messages::MatchPlayMessages command;
                command.set_command(cur_command); // set the players commmand


                // if it is this players turn

                if (cur_command == constants::Command::GENERATE_MOVE) {
                    cur_command = constants::Command::PLAY_MOVE; // update their turn to play an opponents move next
                } else {
                    {
                        std::mutex await_move; // await for a move to be ready
                        std::unique_lock<std::mutex> move_lock(await_move);
                        move_available_c.wait(move_lock, [this] {
                            return move_available || !referee_server_->is_running_;
                        });

                        if (!referee_server_->is_running_) {
                            command.set_command(constants::Command::GAME_TERMINATION);  
                        }

                        cur_command = constants::Command::GENERATE_MOVE;
                        command.mutable_action()->set_x(last_move);
                        move_available = false;
                    }
                    
                }
                
                // send the command to the player
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