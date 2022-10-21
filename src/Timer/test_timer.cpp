//
// Created by epoch on 10/20/22.
//

#include <iostream>

#include <chrono>
#include <atomic>
#include <thread>
#include <functional>


#include <iostream>
using std::cout,std::endl;

template<typename Precision>
class Timer {
public:
    Timer(Precision period, Precision check_interval) : period(period), remain(period),
                                                        check_interval(check_interval) {}

    virtual ~Timer() {
        /* stop the task */
        this->pause = true;
    }

    //TODO add multi parameter extensions.
    void start() {
        reset();
        this->pause = false;
        std::thread t(&Timer::operate, this);
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
    void operate() {
        auto previous_time_point = std::chrono::system_clock::now();
        cout << "I' in " << endl;
        while (!pause) {
            auto now = std::chrono::system_clock::now();
            remain = remain.load() - std::chrono::duration_cast<Precision>(now - previous_time_point);
            if (remain.load().count() < 0) {
                reset();
//                f();
                std::cout << "Hello" << std::endl;
            }
            previous_time_point = now;
            std::this_thread::sleep_for(this->check_interval.load());
        }
    }
};
using namespace  std;

void f() {
    cout << "hello Timer" << endl;
}

void f2(std::function<void(void)> func) {

}

int main() {
    using sec = std::chrono::milliseconds;
    std::atomic<sec> s(sec(-1));
    cout << typeid(s.load().count()).name() << endl;
    Timer<std::chrono::milliseconds> timer(sec(100), sec(10));
//    timer.stop();
    timer.start();
    f2(f);
    this_thread::sleep_for(sec(10000000000));
}