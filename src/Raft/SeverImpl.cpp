//
// Created by epoch on 10/24/22.
//
#include <bits/stdc++.h>
#include "Server.hpp"
#include "buttonrpc.hpp"

using namespace std;
void execute_command(const string& command) {
    cout << "execute command:" + command << endl;
}
int main(int argc, char* argv[]) {
    size_t id = argc >= 2 ? stoul(argv[1]) : 1;
    string ip = "127.0.0.1";
    int port = 5000;
    port += id;

    Server s(id, ip, port);
    s.start_serve();

    return 0;
}