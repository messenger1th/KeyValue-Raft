//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "Timer.hpp"

using namespace std;
struct Data {
    size_t index;
    size_t term;;
};


class Temp {
public:
    void f() {
//        cout << "ehhhh" << endl;
    }
};

using namespace std;

int main() {
    using ms = std::chrono::milliseconds;
    using sec = std::chrono::seconds;
//    ms s1(ms(1));
//    atomic<ms> a(s1);
//
//    Timer<ms> timer(1000);
//
//    Temp t;
//    timer.set_callback(&Temp::f, t);
//    timer.run();
//    std::this_thread::sleep_for(std::chrono::seconds (2));
//    timer.pause();
//    timer.reset();
//    while (true);
    Timer<ms> timer(100);
    Temp t;
    timer.set_callback(&Temp::f, &t);
    timer.run();
    for (int i = 0; i < 1000000000; ++i) {
        timer.reset();
    }


    return 0;
}
