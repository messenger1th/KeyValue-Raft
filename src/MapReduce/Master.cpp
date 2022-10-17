//
// Created by epoch on 10/15/22.
//

#include "Master.hpp"
//TODO delete this head file <bits/stdc++.h>
#include <bits/stdc++.h>
#include "buttonrpc.hpp"

Master::Master(size_t mapper_count, size_t reducer_count, size_t total_map_task_count,
               size_t total_reduce_task_count) :
        mapper_count(mapper_count),
        reducer_count(reducer_count),

        total_map_task_count(total_map_task_count),
        total_reduce_task_count(total_reduce_task_count),

        current_state(mapping),
        mapping_lock(state_mutex),
        shuffling_lock(state_mutex, std::defer_lock),
        reducing_lock(state_mutex, std::defer_lock),
        finalizing_lock(state_mutex, std::defer_lock)
    {
    string task_prefix = "map_task_";
    for (size_t i = 0; i < total_map_task_count; ++i) {
        map_tasks.emplace(task_prefix + to_string(i));
    }
}

std::string Master::assign_map_task(size_t mapper_id) {
    state_change.wait(mapping_lock, [this] () {
       return this->current_state  == mapping;
    });

    if (map_tasks.empty()) {
        return "";
    }
    printf("Mapper: %d is calling assigning map work! \n", mapper_id);
    string task_name = map_tasks.front(); map_tasks.pop();
    return task_name;
}

void Master::map_task_done(size_t id, size_t task_id) {
    //DO Some work: log
    printf("Mapper: %d has done map task: %ld\n", id, task_id);

    ++map_task_done_count;
    if (map_task_done_count == total_map_task_count) {
        change_state(mapping_lock, shuffling, shuffling_lock);
        cout << "map task done!" << endl;

        shuffle();
    }
}


void Master::shuffle() {
    state_change.wait(shuffling_lock, [this] () {
        return this->current_state == shuffling;
    });
    cout << " Shuffling " << endl;
    //TODO shuffle
    change_state(shuffling_lock, reducing, reducing_lock);
    cout << (current_state == reducing) << endl;
}


std::string Master::assign_reduce_task(size_t id) {
    printf("Reducer: %d is calling assigning reduce work! \n", id);

    state_change.wait(reducing_lock, [this] () {
       return this->current_state == reducing;
    });
    return "reduce_task_" + to_string(id);
}

void Master::reduce_task_done(size_t id, size_t task_id) {
    printf("Reducer: %d has done reduce task: %ld\n", id, task_id);

    ++reduce_task_done_count;
    if (reduce_task_done_count == total_reduce_task_count) {
        change_state(reducing_lock, finalizing, finalizing_lock);
        cout << "map task done!" << endl;
    }
}

void Master::finalize() {
    state_change.wait(finalizing_lock, [this] () {
        return this->current_state == finalizing;
    });

    //TODO: finalize
    cout << "!!!All DONE!!!" << endl;
}








