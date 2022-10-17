//
// Created by epoch on 10/17/22.
//
#include "Reducer.hpp"
#include <bits/stdc++.h>
#include <buttonrpc.hpp>
constexpr size_t Infinite = 2000000000;

using namespace std;
void WordAdder::reduce(const std::string key, const std::list<std::string> values) {

}

WordAdder::WordAdder(size_t id): id(id) {}

void WordAdder::start_work() {
    buttonrpc client;
    client.as_client("127.0.0.1", 5555);
    client.set_timeout(Infinite);
    string task_name = client.call<std::string>("assign_reduce_task", this->id).val();
    printf("Reducer: %d is Reducing... task: %s", this->id, task_name.c_str());
}
