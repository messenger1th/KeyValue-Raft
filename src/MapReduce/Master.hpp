//
// Created by epoch on 10/15/22.
//

#ifndef MIT6_824_C_MASTER_HPP
#define MIT6_824_C_MASTER_HPP

#include <cstdio>
#include <string>
#include <queue>

constexpr size_t default_map_task = 10;

class Master {
public:
    Master(size_t mapper_count, size_t reducer_count, size_t total_map_task_count);

    /*called by mapper*/
    std::string assign_map_task(size_t mapper_id); // call for assigning map task;
    void map_task_done(size_t mapper_id, size_t map_task_id); // call for down task;


    /* called by reducer */
    std::string assign_reduce_task();
    void reduce_task_done();


    /* TODO : add check function periodically */
    bool check_map_task();

private:
    size_t mapper_count;
    std::queue<std::string> map_tasks;
    size_t map_task_done_count;
    size_t total_map_task_count = default_map_task;

    size_t reducer_count;
    std::queue<std::string> reduce_tasks;
};


#endif //MIT6_824_C_MASTER_HPP
