//
// Created by epoch on 10/20/22.
//

#ifndef MIT6_824_C_TIMER_HPP
#define MIT6_824_C_TIMER_HPP

#include <chrono>
#include <atomic>
#include <thread>
#include <functional>
#include <iostream>
#include <mutex>
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


    template<class Period>
    void reset_period(Period&& period);

    void reset() {
        if (this->state == State::resetting) {
            return;
        }

        std::cout << "reset: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;
        if (this->state == State::pause) {
            this->remain = period;
        } else {
            this->state = State::resetting;
            running_lock.unlock();
        }
    }

    void stop() {
        std::cout << "stop: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;

        if (this->state == State::running) {
            this->state = State::pause;
            running_lock.unlock();
        }
        this->remain = this->period;
    }

    void pause() {
        if (this->state == State::pause) {
            return;
        }
        std::cout << "pause: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;
        this->state = State::pause;
        running_lock.unlock();
    }

    void run() {
        /* this states will not pause, just return */
        if (this->state == State::running || this->state == State::resetting) {
            return;
        }
        std::cout << "run: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;
        this->state = State::running;
        std::cout << "pause has a lock ? ans : " << pause_lock.owns_lock() << std::endl;
        pause_lock.unlock();
    }


private:

    enum class State{
            running = 1,
            pause = 2,
            shutdown = 3,
            resetting = 4,
    };
    std::atomic<State> state{State::pause};
    Precision period;
    std::atomic<Precision> remain;
    std::atomic<bool> once{false};

    /* use for manage timer start & pause */
    std::timed_mutex m;
    std::condition_variable cv;
    std::unique_lock<std::timed_mutex> running_lock;
    std::unique_lock<std::timed_mutex> pause_lock;

private:
    /* class help functions */
    template<typename Func>
    void operate(Func&& f);
};

template<typename Precision>
Timer<Precision>:: ~Timer() {
    /* stop the detached thread */
    this->shutdown();
}

template<typename Precision>
template<class Period>
Timer<Precision>::Timer(Period &&period) :  period(std::forward<Period>(period)), remain(this->period), pause_lock(m), running_lock(m, std::defer_lock) {}


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
//    running_lock.lock();
    while (this->state != State::shutdown) {
        /* waiting signal to start */
        if (!running_lock.owns_lock()) {
            /* lock() function will block here. */
            running_lock.lock(); // is it possible to make it sleep ?
        }
//        cv.wait(running_lock, [=] () -> bool { return this->state == State::running; });
//        std::cout << "running " << pause_lock.owns_lock() << running_lock.owns_lock()<< std::endl;

        auto start_point = std::chrono::system_clock::now();

//        pause_lock.lock();
//        std::cout << "pass " << pause_lock.owns_lock() << running_lock.owns_lock()<< std::endl;

        /* what's happen when remain.load() < 0 ? */
        if (pause_lock.try_lock_for(remain.load())) {
            if (this->state == State::pause) {
                remain = remain.load() - std::chrono::duration_cast<Precision>(std::chrono::system_clock::now() - start_point);
            } else if (this->state == State::resetting) {
//                std::cout << "resetting" << std::endl;
                std::cout << "pause has a Lock ? ans : " << pause_lock.owns_lock() << std::endl;
                pause_lock.unlock();
                this->state = State::running;
                remain = period;
            }

        } else {
//            std::cout << "timeout" << std::endl;
            f();
            this->remain = period;
        }
    }
}


#endif //MIT6_824_C_TIMER_HPP



