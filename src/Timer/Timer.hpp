//
// Created by epoch on 10/20/22.
//

#ifndef MIT6_824_C_TIMER_HPP
#define MIT6_824_C_TIMER_HPP

#include <chrono>
#include <atomic>
#include <thread>
#include <functional>
#include <condition_variable>


template<typename Precision>
class Timer {
public:

    /* just pass default period & set check interval as 1/10 of period */
    template<class Period>
    explicit Timer(Period&& period);

    /* set stop_flag to true to stop detached thread */
    virtual ~Timer();

    /* pass the callback function & its parameters */
    template<typename Func, typename... Args>
    void set_callback(Func&& f, Args&&... args);

    inline void shutdown() {
        this->state = State::shutdown;
    }


    /* just pass default period & set check interval as 1/10 of period */
    template<class Period>
    void reset_period(Period&& period);

    inline void reset() {
        this->remain = period;
    }

    inline void pause() {
        running_lock.unlock();
        this->state = State::pause;
        cv.notify_one();
    }

    inline void run() {
        pause_lock.unlock();
        this->state = State::runing;
        cv.notify_one();
    }

    inline void set_period(Precision period) {
        this->period = period;
    }

private:

    enum class State{running, pause, shutdown};
    std::atomic<State> state{State::pause};
    Precision period;
    std::atomic<Precision> remain;
    std::atomic<bool> once{false};

    /* use for manage timer start & pause */
    std::mutex m;
    std::condition_variable cv;
    std::unique_lock<std::mutex> running_lock;
    std::unique_lock<std::mutex> pause_lock;

private:
    /* class help functions */
    template<typename Func>
    void operate(Func&& f);
};

template<typename Precision>
Timer<Precision>:: ~Timer() {
    /* stop the detached thread */
    this->state = State::shutdown;
}

template<typename Precision>
template<class Period>
Timer<Precision>::Timer(Period &&period) :  period(std::forward<Period>(period)), remain(this->period), pause_lock(m, std::defer_lock), running_lock(m, std::defer_lock) {}


template<typename Precision>
template<class Period>
void Timer<Precision>::reset_period(Period &&period) {
    this->period = std::forward<Period>(period);
}


template<typename Precision>
template<typename Func, typename... Args>
void Timer<Precision>::set_callback(Func &&f, Args &&... args) {
    /* detect the function type ant pass it the thread constructor */
    using result_type = decltype(std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
    std::thread t(&Timer::operate<result_type>, this, std::bind<void>(std::forward<Func>(f), std::forward<Args>(args)...));
    t.detach();
}

template<typename Precision>
template<typename Func>
void Timer<Precision>::operate(Func&&  f) {
    auto previous_time_point = std::chrono::system_clock::now();
    while (this->state != State::shutdown) {
        /* waiting signal to start */
        cv.wait(running_lock, [=] () -> bool { return this->state == State::running; });
        auto start_point = std::chrono::system_clock::now();
        if (cv.wait_for(pause_lock, remain.load(), [=] () -> bool { return this->state == State::pause; })) {
            remain = remain.load() - std::chrono::duration_cast<Precision>(std::chrono::system_clock::now() - start_point).count();
        } else {
            f();
            this->remain = period;
        }
    }
}

class Temp {
public:
    void f() {

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
    std::this_thread::sleep_for(sec (3));

    timer.pause();

    this_thread::sleep_for(sec (3));

    this_thread::sleep_for(sec (3));

    timer.shutdown();
    this_thread::sleep_for(sec(1));

}

#endif //MIT6_824_C_TIMER_HPP



