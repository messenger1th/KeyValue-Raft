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
#include <cassert>
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
        std::unique_lock<std::mutex> state_lock(state_mutex);
        if (this->state == State::resetting) {
            return;
        }

//        std::cout << "reset: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;
        if (this->state == State::pause) {
            this->remain = period;
        } else if (this->state == State::running) {
            this->state = State::resetting;
            //TODO: fix the bug: it will be unlock a unlocked locker when frequency of calling resetting big enough */
            running_lock.unlock();
        }
    }

    void stop() {
        pause();
        reset();
    }

    void pause() {
        std::unique_lock<std::mutex> state_lock(state_mutex);
        if (this->state == State::pause) {
            return;
        }
        if (state == State::resetting) {
            // todo face thie condition
            this->state
        }
        std::cout << "pause: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;
        this->state = State::pause;
        std::cout << "pause()-> running has a lock ? ans : " << pause_lock.owns_lock() << std::endl;
        running_lock.unlock();
    }

    void run() {
        std::unique_lock<std::mutex> state_lock(state_mutex);


        /* this states will not pause, just return */
        if (this->state == State::running || this->state == State::resetting) {
            return;
        }
//        std::cout << "run: " << static_cast<std::underlying_type_t<State>>(state.load()) << std::endl;
        this->state = State::running;
        std::cout << "run()  pause has a lock ? ans : " << pause_lock.owns_lock() << std::endl;
        pause_lock.unlock();
    }


private:

    enum class State{
            running = 1,
            pause = 2,
            shutdown = 3,
            resetting = 4,
    };
    std::mutex state_mutex;
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
            running_lock.lock(); //TODO: figure out is it possible to make it sleep ?
        }

        this->state = State::running;
        auto start_point = std::chrono::system_clock::now();

        //TODO what's happen when remain.load() < 0 ?
        if (pause_lock.try_lock_for(remain.load())) {
            std::unique_lock<std::mutex> state_lock(state_mutex);
            if (this->state == State::pause) { /* pause lock */
                remain = remain.load() - std::chrono::duration_cast<Precision>(std::chrono::system_clock::now() - start_point);
            } else if (this->state == State::resetting) { /* with pause locker but will unlocker later, and running lock*/
//                std::cout << "resetting" << std::endl;
//                std::cout << "pause has a Lock ? ans : " << pause_lock.owns_lock() << std::endl;
                pause_lock.unlock();
                remain = period;
            }
        } else { /* with running locker */
            f();
            this->remain = period;
        }
    }
}


#endif //MIT6_824_C_TIMER_HPP



