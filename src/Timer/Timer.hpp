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

    template<class Period, class IntervalType>
    Timer(Period&& period, IntervalType&& check_interval);
    Timer(Precision period, Precision check_interval);

    virtual ~Timer();

    template<typename Func, typename... Args>
    void start(Func&& f, Args&&... args);

    inline void stop() {
        this->pause = true;
    }

    inline void reset() {
        this->remain = period;
    }

    inline void reset_check_interval(Precision interval) {
        this->check_interval = interval;
    }


private:
    std::atomic<bool> pause{false};
    Precision period;
    std::atomic<Precision> remain;
    std::atomic<Precision> check_interval;

private:
    template<typename Func>
    void operate(const Func& f);
};

template<typename Precision>
Timer<Precision>:: ~Timer() {
    /* stop the detached thread */
    this->pause = true;
}

template<typename Precision>
template<class Period, class IntervalType>
Timer<Precision>::Timer(Period &&period, IntervalType &&check_interval) :  period(std::forward<Period>(period)),
                                                                           remain(this->period),
                                                                           check_interval(Precision(std::forward<IntervalType>(check_interval))) {}

template<typename Precision>
Timer<Precision>::Timer(Precision period, Precision check_interval)  : period(period), remain(period),
                                                                       check_interval(check_interval) {}

template<typename Precision>
template<typename Func, typename... Args>
void Timer<Precision>::start(Func &&f, Args &&... args) {
    reset();
    this->pause = false;
    /* detect the function type ant pass it the thread constructor */
    using operation_type = decltype(std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
    std::thread t(&Timer::operate<operation_type>, this, std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
    t.detach();
}

template<typename Precision>
template<typename Func>
void Timer<Precision>::operate(const Func &f) {
    auto previous_time_point = std::chrono::system_clock::now();
    while (!pause) {
        auto now = std::chrono::system_clock::now();
        remain = remain.load() - std::chrono::duration_cast<Precision>(now - previous_time_point);
        if (remain.load() < remain.load().zero()) {
            reset();
            /*use  function bounded in function start()*/
            f();
        }
        previous_time_point = now;
        std::this_thread::sleep_for(this->check_interval.load());
    }
}


#endif //MIT6_824_C_TIMER_HPP
