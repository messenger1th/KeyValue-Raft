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
    if (map_tasks.empty() || current_state != mapping) {
        return "";
    }

    state_change.wait(mapping_lock, [this] () {
       return this->current_state  == mapping;
    });


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

    string input_prefix = "map_result_";
    string intermediate_prefix = "reduce_unordered_task_";

    ifstream reader;
    vector<ofstream> writers(this->reducer_count);

    for (int i = 0; i < writers.size(); ++i) {
        writers[i].open(intermediate_prefix + to_string(i));
        assert(writers[i].is_open());
    }


    hash<std::string>   reduce_task_separator;
    for (int i = 0; i < total_map_task_count; ++i) {
        string input_name = input_prefix + to_string(i);
        reader.open(input_name);
        assert(reader.is_open());

        string kv;

        while (std::getline(reader, kv)) {
            auto pos = kv.find(':');
            const string& key = kv.substr(0, pos);
            size_t task_number = reduce_task_separator(key) % this->reducer_count;
            writers[task_number] << kv << '\n';
        }
        reader.close();
    }
    
    for_each(writers.begin(), writers.end(), [] (auto& writer) {
        writer.close();
    });
    
    string output_prefix = "reduce_task_";
    ofstream writer;
    for (int i = 0; i < this->reducer_count; ++i) {
        /* read from intermediate file */
        string intermediate_name = intermediate_prefix + to_string(i);
        vector<string> key_values;
        string one_line;
        reader.open(intermediate_name); assert(reader.is_open());
        while (std::getline(reader, one_line)) {
            key_values.emplace_back(one_line);
        }
        reader.close();

        /* use external sort  when data is over size */
        sort(key_values.begin(), key_values.end());

        /*after sort, output to the reduce input*/
        string output_name = output_prefix + to_string(i);
        writer.open(output_name); assert(writer.is_open());
        for (const auto& kv: key_values) {
            writer << kv << '\n';
        }
        writer.close();
    }
        
    change_state(shuffling_lock, reducing, reducing_lock);
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








