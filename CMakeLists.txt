cmake_minimum_required(VERSION 3.9)
project(MIT6.824-C++ CXX)
set(CMAKE_CXX_STANDARD 17)

include_directories(src)
include_directories(src/TinyRPC)
include_directories(str/Timer)

add_subdirectory(src/Raft Raft)
add_subdirectory(src/Timer Timer)
add_subdirectory(src/TinyRPC TinyRPC)
add_subdirectory(src/MapReduce MapReduce)



#target_include_directories(client PUBLIC
#        "${PROJECT_BINARY_DIR}"
#        "${PROJECT_SOURCE_DIR}/src"
#        )
