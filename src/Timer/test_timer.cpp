//
// Created by epoch on 10/20/22.
//

#include <iostream>
#include <chrono>
#include <atomic>
#include <thread>
#include <functional>
//#include "Timer.hpp"


using std::cout,std::endl;
using namespace  std;

template<typename Precision>
class Timer {
public:
    Timer(Precision period, Precision check_interval) : period(period), remain(period),
                                                        check_interval(check_interval) {}

    virtual ~Timer() {
        /* stop the detached thread */
        this->pause = true;
    }

    //TODO add multi parameter extensions.
    template<typename Func, typename... Args>
    void start(Func&& f, Args&&... args) {
        reset();
        this->pause = false;
        /* detect the function type ant pass it the thread constructor */
        using operation_type = decltype(std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
        std::thread t(&Timer::operate<operation_type>, this, std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
        t.detach();
    }

    void stop() {
        this->pause = true;
    }

    void reset() {
        this->remain = period;
    }

    void reset_check_interval(Precision interval) {
        this->check_interval = interval;
    }


private:
    std::atomic<bool> pause{false};
    Precision period;
    std::atomic<Precision> remain;
    std::atomic<Precision> check_interval;

private:
    //TODO: iterate it for multi parameters;
    template<typename Func>
    void operate(const Func& f) {
        auto previous_time_point = std::chrono::system_clock::now();
        cout << "I' in " << endl;
        while (!pause) {
            auto now = std::chrono::system_clock::now();
            remain = remain.load() - std::chrono::duration_cast<Precision>(now - previous_time_point);
            if (remain.load() < remain.load().zero()) {
                reset();
                f();
            }
            previous_time_point = now;
            std::this_thread::sleep_for(this->check_interval.load());
        }
        cout << "stopped" << endl;
    }
};



//void f(int i) {
//    printf("f(%d)\n", i);
//}

void f(int a, int b) {
    printf("f(%d, %d)\n", a, b);
}


int main() {
    using ms = std::chrono::milliseconds;
    using sec = std::chrono::seconds;
    ms s2; s2.zero();
    std::atomic<ms> s(ms(-1));
    cout << typeid(s.load().count()).name() << endl;
    Timer<ms> timer(ms(1000), ms(10));

    timer.start(f, 10, 20);
    this_thread::sleep_for(sec (5));

    timer.stop();
    this_thread::sleep_for(sec(1));
}