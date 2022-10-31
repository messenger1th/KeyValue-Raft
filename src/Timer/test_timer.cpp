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
        cout << "ehhhh" << endl;
    }
};

using namespace std;
int main() {
    cout << "Hello" << endl;
    using ms = std::chrono::milliseconds;
    using s = std::chrono::seconds;
//    ms s1(ms(1));
//    atomic<ms> a(s1);
//
//    Timer<ms> timer(1000);
//
//    Temp t;
//    timer.set_callback(&Temp::f, t);
//    timer.run();
//    for (int i = 0; i < 3; ++i) {
//        timer.restart();
//        std::this_thread::sleep_for(ms(500));
//    }
//    std::this_thread::sleep_for(s(3));
//    timer.pause();
//    while (true);
//    Timer<ms> timer(100);
//    Temp t;
//    timer.set_callback(&Temp::f, &t);
//    timer.run();



    std::mutex m;
//    std::unique_lock<std::mutex> lock_before(m);
    bool flag = false;
    std::condition_variable cv;
    std::unique_lock<std::mutex> lock_before(m, std::defer_lock);
    std::thread t([&]() {
        std::this_thread::sleep_for(s(2));
//        cv.notify_one();
//        lock_before.unlock();
        cout << "unlocked" << endl;
        flag = true;
//        cv.notify_one();
    }); t.detach();

    cout << "I'm not lock" << endl;
    std::unique_lock<std::mutex> lock(m, std::defer_lock);
    lock.lock();
    cout << "locked" << endl;
//    cv.w

    cout << cv.wait_for(lock, s(5), [] () {
        return false;
    });
    cout << "get" << endl;
    lock.unlock();
    cout << lock.owns_lock() << endl;
    return 0;
}
