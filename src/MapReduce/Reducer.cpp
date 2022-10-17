//
// Created by epoch on 10/17/22.
//
#include "Reducer.hpp"
#include <bits/stdc++.h>
#include <buttonrpc.hpp>
constexpr size_t Infinite = 2000000000;

using namespace std;
void WordAdder::reduce(const std::string& key, const std::list<std::string>& values) {
    size_t count = 0;
    for (const auto& value: values) {
        count += stoul(value);
    }
    write_disk(key, count);
    reader.close();
}

WordAdder::WordAdder(size_t id): id(id) {}

void WordAdder::start_work() {
    buttonrpc client;
    client.as_client("127.0.0.1", 5555);
    client.set_timeout(Infinite);

    /*spine lock*/
    string task_name = client.call<std::string>("assign_reduce_task", this->id).val();
    while (task_name == "") {
        task_name = client.call<std::string>("assign_reduce_task", this->id).val();
        sleep(4);
    }
    printf("Reducer: %d is Reducing... task: %s\n", this->id, task_name.c_str());
    reader.open(task_name);
    string key;
    list<std::string> values;
    string next_line;
    while (std::getline(reader, next_line)) {
        auto pos = next_line.find(':');
        assert(pos != std::string::npos);
        const string& next_key = next_line.substr(0, pos - 0);
        if (values.empty()) {
            key = next_key;
        } else if (key != next_key) {
            this->reduce(key, values);
            key = next_key;
            values.clear();
        }
        cout << next_key << endl;
        values.emplace_back(next_line.substr(pos + 1));
    }
    printf("Reducer: %d done task: %s\n", this->id, task_name.c_str());
    client.call<void>("reduce_task_done", this->id, this->id);
    reader.close();
    exit(0);
}

void WordAdder::write_disk(const string &key, const size_t &value) {
    //do something to disk
    printf("key: %s, value: %d\n", key.c_str(), value);
}
