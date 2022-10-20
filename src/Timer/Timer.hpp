//
// Created by epoch on 10/20/22.
//

#ifndef MIT6_824_C_TIMER_HPP
#define MIT6_824_C_TIMER_HPP

#include <chrono>
#include <atomic>
#include <thread>
#include <functional>

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
    void start(std::function<void(void)> f) {
        reset();
        this->pause = false;
        std::thread t(this->operate, f);
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
    void operate(std::function<void(void)> f) {
        auto previous_time_point = std::chrono::system_clock::now();
        while (!pause) {
            auto now = std::chrono::system_clock::now();
            remain -= now - previous_time_point;
            if (remain < 0) {
                reset();
                f();
            }
            previous_time_point = now;
            std::this_thread::sleep_for(this->check_interval);
        }
    }
};



#endif //MIT6_824_C_TIMER_HPP
