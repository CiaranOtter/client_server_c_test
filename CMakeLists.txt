cmake_minimum_required(VERSION 3.20)
project(server_test)

find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(MPI REQUIRED)

add_library(protolib proto/game_comm.proto)

target_link_libraries(protolib gRPC::grpc++)

target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

protobuf_generate(TARGET protolib LANGUAGE cpp)

protobuf_generate(TARGET protolib LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
)

# add_executable(server src/comm_server.cc)
# target_link_libraries(server protolib)

add_executable(comm_server src/comm_server.cc)
target_link_libraries(comm_server protolib)
target_link_libraries(comm_server gRPC::grpc++_reflection)

add_executable(comm_player src/comm_client.cc src/player.c)
target_link_libraries(comm_player MPI::MPI_CXX)
target_link_libraries(comm_player protolib)

