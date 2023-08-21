//
// Created by epoch on 10/24/22.
//
#include <bits/stdc++.h>
#include "Raft.hpp"
#include "buttonrpc.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    size_t id = argc >= 2 ? stoul(argv[1]) : 1;
    string ip = "127.0.0.1";
    int port = 5000;
    port += id;

    Raft s(id, ip, port);
    s.start_service();

    return 0;
}