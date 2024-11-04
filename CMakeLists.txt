cmake_minimum_required(VERSION 3.20)
project(server_test)

# Find the protobuf tools package 
find_package(Protobuf CONFIG REQUIRED)
# find the grpc tools package 
find_package(gRPC CONFIG REQUIRED)
# include MPI for C file 
find_package(MPI REQUIRED)

# Include the directory for the protobuf api
include_directories(tournament_engine_API)

# Get the files from the proto library 
file(GLOB PROTO_SOURCES "proto/*.proto")

# Add the library to the project
add_library(protolib ${PROTO_SOURCES})
# add_library(utilprotolib tournament_engine_API/util/util.proto)
# add_library(protolib tournament_engine_API/game_comm/game_comm.proto)
# add_library(agentprotolib tournament_engine_API/agent/agent.proto)

# target_link_libraries(utilprotolib  gRPC::grpc++)
target_link_libraries(protolib gRPC::grpc++)
# target_link_libraries(agentprotolib gRPC:grpc++)

# target_include_directories(utilprotolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_include_directories(protolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
# target_include_directories(agentprotolib PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

get_target_property(grpc_cpp_plugin_location gRPC::grpc_cpp_plugin LOCATION)

# protobuf_generate(TARGET utilprotolib LANGUAGE cpp)
protobuf_generate(
    TARGET protolib
    LANGUAGE cpp
    IMPORT_DIRS ./proto
)
# protobuf_generate(TARGET agentprotolib LANGUAGE cpp)

# protobuf_generate(TARGET utilprotolib LANGUAGE grpc
#     GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
#     PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
# )
protobuf_generate(TARGET protolib LANGUAGE grpc
    GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
    PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
    IMPORT_DIRS ./proto
)
# protobuf_generate(TARGET agentprotolib LANGUAGE grpc
#     GENERATE_EXTENSIONS .grpc.pb.h .grpc.pb.cc
#     PLUGIN "protoc-gen-grpc=${grpc_cpp_plugin_location}"
# )

# add_executable(server src/comm_server.cc)
# target_link_libraries(server protolib)

add_executable(comm_server src/comm_server.cc)
# target_link_libraries(comm_server utilprotolib)
target_link_libraries(comm_server protolib)
# target_link_libraries(comm_server agentprotolib)
target_link_libraries(comm_server gRPC::grpc++_reflection)

add_executable(comm_player src/comm_client.cc src/player.c)
target_link_libraries(comm_player MPI::MPI_CXX)
target_link_libraries(comm_player protolib)


## try out adding grpc to the project using github

# cmake_minimum_required(VERSION 3.15)
# project(my_exe)

# include(FetchContent)

# FetchContent_Declare(
#   gRPC
#   GIT_REPOSITORY https://github.com/grpc/grpc
#   GIT_TAG        v1.28.0
#   )
# set(FETCHCONTENT_QUIET OFF)
# FetchContent_MakeAvailable(gRPC)

# add_executable(my_exe my_exe.cc)
# target_link_libraries(my_exe grpc++)

