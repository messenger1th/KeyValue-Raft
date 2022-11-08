cmake_minimum_required(VERSION 2.12)


set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/kvRaft/)


add_executable(kvRaft SkipList.hpp kvRaft.cpp ../Raft/Raft.cpp ../Raft/Raft.hpp)
target_link_libraries(kvRaft TinyRPC)


add_definitions(-w)