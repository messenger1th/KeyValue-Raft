cmake_minimum_required(VERSION 2.12)
include_directories(.)


set_target_properties(TinyRPC PROPERTIES LINKER_LANGUAGE CXX)
add_subdirectory(test MapReduce/test)

#add_executable(master Master.cpp)
#add_executable(mapper Mapper.cpp)
#target_link_libraries(master TinyRPC)
#target_link_libraries(mapper TinyRPC)