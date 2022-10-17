//
// Created by epoch on 10/15/22.
//

#ifndef MIT6_824_C_MASTER_HPP
#define MIT6_824_C_MASTER_HPP

#include <cstdio>
#include <string>
#include <queue>
#include <iostream>
#include <condition_variable>


constexpr size_t default_map_task = 10;

class Master {
public:

    /* honestly, there are another state like Input Splitting, finalizing */
    enum state {mapping, shuffling, reducing, finalizing};

    Master(size_t mapper_count, size_t reducer_count, size_t total_map_task_count,
           size_t total_reduce_task_count);

    /*locally do by master which should be shuffler done*/
    void shuffle();

    /*called by mapper*/
    std::string assign_map_task(size_t mapper_id); // call for assigning map task;
    void map_task_done(size_t mapper_id, size_t map_task_id); // call for down task;


    /* called by reducer */
    std::string assign_reduce_task(size_t id);
    void reduce_task_done(size_t id, size_t task_id);


    void finalize();


    /* TODO : add check function periodically */
    bool check_map_task();

private:
    state current_state;
    std::mutex state_mutex;
    std::condition_variable state_change;
    std::unique_lock<std::mutex> mapping_lock;
    std::unique_lock<std::mutex> shuffling_lock;
    std::unique_lock<std::mutex> reducing_lock;
    std::unique_lock<std::mutex> finalizing_lock;


    size_t mapper_count;
    std::queue<std::string> map_tasks;
    size_t map_task_done_count;
    size_t total_map_task_count = default_map_task;


    size_t reducer_count;
    size_t reduce_task_done_count;
    size_t total_reduce_task_count;
    std::queue<std::string> reduce_tasks;


private:
    /*function here*/
    void change_state(std::unique_lock<std::mutex>& previous_lock, const state& new_state, std::unique_lock<std::mutex>& new_lock) {
        previous_lock.unlock();
        this->current_state = new_state;
        new_lock.lock();
        state_change.notify_all();
    }
};


#endif //MIT6_824_C_MASTER_HPP
