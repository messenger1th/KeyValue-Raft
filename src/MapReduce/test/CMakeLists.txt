set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin/MapReduce/)

add_executable(test_master test_master.cpp ../Master.cpp)
add_executable(test_mapper test_mapper.cpp ../Mapper.cpp)
add_executable(test_reducer test_reducer.cpp ../Reducer.cpp)

target_link_libraries(test_master PUBLIC TinyRPC)
target_link_libraries(test_mapper PUBLIC TinyRPC)
target_link_libraries(test_reducer PUBLIC TinyRPC)