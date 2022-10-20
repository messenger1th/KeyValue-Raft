//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include "Timer.hpp"
using namespace  std;

void f() {
    cout << "hello Timer" << endl;
}

void f2(std::function<void(void)> func) {

}

int main() {
    using sec = std::chrono::milliseconds;
    Timer<std::chrono::milliseconds> timer(sec(100), sec(10));
    timer.start(f);
    f2(f);
}