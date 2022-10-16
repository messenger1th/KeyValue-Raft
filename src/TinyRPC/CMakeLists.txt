cmake_minimum_required(VERSION 2.12)
include_directories(.)


#set(LIBRARY_OUTPUT_PATH "${PROJECT_SOURCE_DIR}/bin")
add_library(TinyRPC STATIC buttonrpc.hpp)
set_target_properties(TinyRPC PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(TinyRPC PUBLIC zmq)


# test client & test server
add_executable(client example/main_client.cpp)
add_executable(server example/main_server.cpp)
target_link_libraries(client PUBLIC TinyRPC)
target_link_libraries(server PUBLIC TinyRPC)