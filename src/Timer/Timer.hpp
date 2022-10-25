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

    /* just pass default period & set check interval as 1/10 of period */
    template<class Period>
    Timer(Period&& period);
    
    /* more readable constructor, just pass the value*/
    template<class Period, class IntervalType>
    Timer(Period&& period, IntervalType&& check_interval);

    /* initial constructor, pass by the corresponding type */
    Timer(Precision period, Precision check_interval);

    /* set stop_flag to true to stop detached thread */
    virtual ~Timer();

    /* pass the callback function & its parameters */
    template<typename Func, typename... Args>
    void start(Func&& f, Args&&... args);

    inline void stop() {
        this->stop_flag = true;
    }

    inline void reset() {
        this->remain = period;
    }

    inline void puase() {
        this->puase = true;
    }

    inline void resume() {
        this->puase = true;
    }

    inline void reset_check_interval(Precision interval) {
        this->check_interval = interval;
    }

    inline void set_once(bool once) {
        this->once = once;
    }

    inline void set_period(Precision period) {
        this->period = period;
    }

private:
    std::atomic<bool> stop_flag{false};
    Precision period;
    std::atomic<Precision> remain;
    std::atomic<Precision> check_interval;
    std::atomic<bool> once{true};
    std::atomic<bool> pause{false};

private:
    /* class help functions */
    template<typename Func>
    void operate(Func&& f);
};

template<typename Precision>
Timer<Precision>:: ~Timer() {
    /* stop the detached thread */
    this->stop_flag = true;
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
template<class Period>
Timer<Precision>::Timer(Period &&period): Timer(period, period / 100) {}

template<typename Precision>
template<typename Func, typename... Args>
void Timer<Precision>::start(Func &&f, Args &&... args) {
    reset();
    this->stop_flag = false;
    /* detect the function type ant pass it the thread constructor */
    using result_type = decltype(std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
    std::thread t(&Timer::operate<result_type>, this, std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
    t.detach();
}

template<typename Precision>
template<typename Func>
void Timer<Precision>::operate(Func&&  f) {
    auto previous_time_point = std::chrono::system_clock::now();
    while (!stop_flag) {
        auto now = std::chrono::system_clock::now();
        //TODO: optimize it, let the thread sleep until get the lock.
        if (!pause) {
            remain = remain.load() - std::chrono::duration_cast<Precision>(now - previous_time_point);
        }
        if (remain.load() < remain.load().zero()) {
            reset();

            /* call function bounded in function start()*/
            f();

            /* if set once, break this loop and exit this thread. */
            if (this->once) {
                break;
            }
        }
        previous_time_point = now;
        std::this_thread::sleep_for(this->check_interval.load());
    }
}



#endif //MIT6_824_C_TIMER_HPP



