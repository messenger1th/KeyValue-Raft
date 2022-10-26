//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include <chrono>
#include <thread>
#include "Timer.hpp"

using namespace std;
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
    ms s1(ms(1));
    atomic<ms> a(s1);

    Timer<ms> timer(1000);

    Temp t;
    timer.set_callback(&Temp::f, t);
    timer.run();
    std::this_thread::sleep_for(std::chrono::seconds (2));
    timer.pause();
    timer.reset();
    while (true);
}
