cmake_minimum_required(VERSION 2.12)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/Raft/)

add_executable(raft_server SeverImpl.cpp Raft.cpp)
target_link_libraries(raft_server TinyRPC)

add_definitions(-w)