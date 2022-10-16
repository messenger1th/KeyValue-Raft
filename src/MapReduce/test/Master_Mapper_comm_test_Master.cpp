//
// Created by epoch on 10/16/22.
//

#include <bits/stdc++.h>
#include "buttonrpc.hpp"
#include "Master.hpp"

using namespace std;

int main() {
    Master master(2, 2, 2);

    int port = 5555;
    buttonrpc server;

    server.as_server(port);
    server.bind("assign_map_task", &Master::assign_map_task, &master);
    server.bind("map_task_done", &Master::map_task_done, &master);

    printf("master is listening port : %d", port);
    cout << endl;
    server.run();
}