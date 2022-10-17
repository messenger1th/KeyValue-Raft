//
// Created by epoch on 10/15/22.
//

#include "Mapper.hpp"
#include <bits/stdc++.h>
#include "buttonrpc.hpp"
using namespace std;


WordCounter::WordCounter(size_t id): id(id) {}


void WordCounter::map(const std::string &key, const std::string &value) {
    std::unordered_map<std::string, size_t> frequency;
    const size_t  n = value.size();
    for (size_t i = 0; i < n; ) {
        /* skip past leading whitespace */
        while (i < n && !isalpha(value[i])) {
            ++i;
        }
        size_t previous = i;
        while (i < n && isalpha(value[i])) {
            ++i;
        }
        if (previous < i)
            ++frequency[value.substr(previous, i - previous)];
    }

    assert(key.find('_') != string::npos);
    string output_file = "map_result_" + key.substr(key.rfind('_') + 1);
    writer.open(output_file);
    assert(writer.is_open());

    for (const auto& p : frequency) {
        const auto k = p.first;
        const auto freq = p.second;
        writer << k << ':' << freq << '\n';
    }
    writer.flush();
    writer.close();
}


void WordCounter::start_work() {
    buttonrpc client;
    client.as_client("127.0.0.1", 5555);
    client.set_timeout(2000);

    string task_name;
    while (true) {
        task_name = client.call<std::string>("assign_map_task", this->id).val();
        if (task_name.empty()) {
            break;
        }
        this->reader.open(task_name);
        assert(reader.is_open());
        this->map(task_name, string(std::istreambuf_iterator<char>(reader), std::istreambuf_iterator<char>()));
        reader.close();
        printf("Mapper: %d finished map task: %s\n", this->id, task_name.c_str());
        client.call<void>("map_task_done", this->id, stoul(task_name.substr(task_name.rfind('_') + 1)));
    }
    exit(0);
}

