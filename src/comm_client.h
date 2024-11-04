// #ifdef __cplusplus
//     extern "C" extern
// #endif
// void test(int test);


#if defined(__cplusplus)

// extern std::shared_ptr<grpc::ClientReaderWriter<Move, Move>> referee;
// extern std::shared_ptr<grpc::Channel> channel;
// extern std::unique_ptr<GameComm::Stub> stub;

#define EXTERNC extern "C"
#else
#define EXTERNC 

enum MessageType {
    GAME_TERMINATION,
    GENERATE_MOVE,
    PLAY_MOVE,
    MATCH_RESET,
    RECV_FAIL,
    CLIENT_DISCONNECTED,
    UNKNOWN,
    GET_COMMAND,
    CONNECT
};
#endif

    EXTERNC const char* playername;

    EXTERNC int initialise_comms(char *address, int port);
    EXTERNC int send_move(char *move);
    EXTERNC int receive_message(int *);
    EXTERNC void close_comms(void);
