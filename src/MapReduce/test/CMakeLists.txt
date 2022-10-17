set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/src/MapReduce/test)

add_executable(test_master test_Master.cpp ../Master.cpp)
add_executable(test_mapper test_Mapper.cpp ../Mapper.cpp)
target_link_libraries(test_master PUBLIC TinyRPC)
target_link_libraries(test_mapper PUBLIC TinyRPC)