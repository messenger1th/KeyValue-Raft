cmake_minimum_required(VERSION 2.12)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/KVRaft/)


add_executable(kvRaft SkipList.h main.cpp)
target_link_libraries(kvRaft TinyRPC)
