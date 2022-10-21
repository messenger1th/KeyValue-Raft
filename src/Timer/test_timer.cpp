//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <functional>
#include "Timer.hpp"

using namespace std;


void f(int a, int b) {
    printf("f(%d, %d)\n", a, b);
}


int main() {
    using ms = std::chrono::milliseconds;
    using sec = std::chrono::seconds;
    ms s2;

    Timer<ms> timer(1000, 100);

    timer.start(f, 10, 20);
    this_thread::sleep_for(sec (3));

    timer.stop();

    this_thread::sleep_for(sec (3));
    timer.start(f, 10, 20);
    this_thread::sleep_for(sec (3));

    timer.stop();
    this_thread::sleep_for(sec(1));

}