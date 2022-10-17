//
// Created by epoch on 10/17/22.
//

#include "Reducer.hpp"
#include <bits/stdc++.h>
#include "buttonrpc.hpp"



using namespace std;

int main(int argc, const char* argv[]) {
    int id = argc < 2 ? 0 : stoi(argv[1]);
    WordAdder reducer(id);
    reducer.start_work();
}

