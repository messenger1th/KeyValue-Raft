cmake_minimum_required(VERSION 3.9)
project(MIT6.824-C++ CXX)
set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(src/TinyRPC)
include_directories(src/Timer)
include_directories(src/Raft)

add_subdirectory(src/TinyRPC TinyRPC)
add_subdirectory(src/Timer Timer)
add_subdirectory(src/Raft Raft)
add_subdirectory(src/kvRaft kvRaft)



#target_include_directories(client PUBLIC
#        "${PROJECT_BINARY_DIR}"
#        "${PROJECT_SOURCE_DIR}/src"
#        )
