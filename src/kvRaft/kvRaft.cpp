#include <iostream>
#include <climits>
#include <cassert>
#include <unordered_map>
#include <chrono>
#include <thread>
#include <algorithm>
#include "SkipList.hpp"
using namespace std;


int main(int argc, const char** argv) {
    size_t id = argc >= 2 ? stoul(argv[1]) : 1;
    string ip = "127.0.0.1";
    int port = 5000;
    port += id;

    SkipList<int, int> kvRaft(id, ip, port);
    kvRaft.start_service();

    return 0;
}
