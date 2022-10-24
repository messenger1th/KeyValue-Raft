//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "Timer.hpp"

using namespace std;


void f(int a, int b) {
    printf("f(%d, %d)\n", a, b);
}


class Temp {
public:
    void f() {

    }
};

using namespace std;

int main() {
    using ms = std::chrono::milliseconds;
    using sec = std::chrono::seconds;
    ms s2;

    Timer<ms> timer(1000, 10);

    timer.set_once(false);
    Temp t;
    timer.start(&Temp::f, t);
    std::this_thread::sleep_for(sec (3));

    timer.stop();

    this_thread::sleep_for(sec (3));

    this_thread::sleep_for(sec (3));

    timer.stop();
    this_thread::sleep_for(sec(1));

}