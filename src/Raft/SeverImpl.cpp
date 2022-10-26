//
// Created by epoch on 10/24/22.
//
#include <bits/stdc++.h>
#include "Server.hpp"
#include "buttonrpc.hpp"

using namespace std;

int main(int argc, char* argv[]) {
    size_t id = argc >= 2 ? stoul(argv[1]) : 1;
    string ip = "127.0.0.1";
    int port = 5000;
    port += id;

    Server s(id, ip, port);
    s.read_config();

    buttonrpc server_rpc;
    server_rpc.as_server(port);
    server_rpc.bind("Hello", &Server::Hello, &s);
    server_rpc.bind("request_vote", &Server::request_vote, &s);
    server_rpc.bind("append_entries", &Server::append_entries, &s);

    s.starts_up();
//    thread t(&Server::starts_up, &s); t.detach();

    /* help to start all server*/
    server_rpc.run();
    return 0;
}