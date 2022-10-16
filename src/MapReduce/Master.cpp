//
// Created by epoch on 10/15/22.
//

#include "Master.hpp"
//TODO delete this head file <bits/stdc++.h>
#include <bits/stdc++.h>
#include "buttonrpc.hpp"

Master::Master(size_t mapper_count, size_t reducer_count, size_t total_map_task_count) :
    mapper_count(mapper_count),
    reducer_count(reducer_count),
    total_map_task_count(total_map_task_count) {


    string task_prefix = "map_task_";
    for (size_t i = 0; i < total_map_task_count; ++i) {
        map_tasks.emplace(task_prefix + to_string(i));
    }
}

std::string Master::assign_map_task(size_t mapper_id) {
    if (map_tasks.empty()) {
        return "";
    }
    printf("Mapper: %d is calling assigning map work! \n", mapper_id);
    string task_name = map_tasks.front(); map_tasks.pop();
    return task_name;
}

void Master::map_task_done(size_t mapper_id, size_t map_task_id) {
    //DO Some work: log
    printf("Mapper: %d has done map task: %ld\n", mapper_id, map_task_id);

    ++map_task_done_count;
    if (map_task_done_count == total_map_task_count) {
        //TODO: set reduce task is running & Notice the reducer.
        cout << "map task done!" << endl;
    }
}



