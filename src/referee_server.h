// referee_server.h
#pragma once

#include <grpcpp/grpcpp.h>
#include "match_play.grpc.pb.h"
#include "messages.pb.h"
#include "constants.pb.h"
#include "stream.grpc.pb.h"
#include <memory>
#include <queue>
#include <mutex>
#include <condition_variable>

class GameLogicInterface {
public:
    virtual ~GameLogicInterface() = default;
    virtual bool validateAction(const messages::Action& action) = 0;
    virtual bool processAction(const messages::Action& action) = 0;
    virtual messages::Match getMatchState() = 0;
    virtual messages::Result getFinalResults() = 0;
};

class RefereeServer {
public:
    RefereeServer(const std::string& game_server_address);
    void RegisterGameLogic(std::unique_ptr<GameLogicInterface> game_logic);
    void Run(const std::string& server_address);
    void Stop();

private:
    class GameServiceImpl final : public match_play::MatchPlayService::Service {
    public:
        GameServiceImpl(RefereeServer* server);
        grpc::Status PlayGame(
            grpc::ServerContext* context,
            grpc::ServerReaderWriter<messages::MatchPlayMessages, 
                                   messages::MatchPlayMessages>* stream) override;

    private:
        RefereeServer* referee_server_;
        int player_count;
        bool move_available;
        std::condition_variable game_ready;
        std::mutex game_ready_m;
        std::condition_variable move_available_c;
    };

     // Methods for handling stream service communication
    void ConnectToStreamService();
    void StartStreamingSender();
    bool SendStreamMessage(const messages::StreamMessages& message);
    
    // Queue management
    void QueueStreamMessage(messages::StreamMessages&& message);
    messages::StreamMessages WaitForNextMessage();

    std::unique_ptr<GameLogicInterface> game_logic_;
    std::unique_ptr<grpc::Server> server_;
    
    // Stream service connection
    std::unique_ptr<stream::StreamService::Stub> stream_service_stub_;
    std::shared_ptr<grpc::Channel> stream_service_channel_;
    std::unique_ptr<grpc::ClientContext> stream_context_;
    std::unique_ptr<grpc::ClientWriter<messages::StreamMessages>> stream_writer_;
    
    // Thread-safe message queue
    std::queue<messages::StreamMessages> message_queue_;
    std::mutex queue_mutex_;
    std::condition_variable queue_cv_;
    std::atomic<bool> is_running_{false};
    std::string stream_service_address_;
};