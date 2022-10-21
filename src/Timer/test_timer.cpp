//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include "Timer.hpp"
using std::cout,std::endl;


using namespace  std;

void f() {
    cout << "hello Timer" << endl;
}



int main() {
    using ms = std::chrono::milliseconds;
    using sec = std::chrono::seconds;
    ms s2; s2.zero();
    std::atomic<ms> s(ms(-1));
    cout << typeid(s.load().count()).name() << endl;
    Timer<ms> timer(ms(1000), ms(10));

    timer.start(f);
    this_thread::sleep_for(sec (5));

    timer.stop();
    this_thread::sleep_for(sec(1));
}