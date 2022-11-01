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
        std::unique_lock<std::mutex> state_lock(state_mutex);
        this->state_changed = true;
        this->state = State::shutdown;
    }


    template<class Period>
    void reset_period(Period&& period);

    /* reset timer & rerun */
    void restart() {
        if (this->state == State::resetting) {
            return;
        }

        {
            std::unique_lock<std::mutex> state_lock(state_mutex);
            this->state = State::resetting;
        }
        this->state_changed = true;
        cv.notify_one();
    }

    /*TODO: implement functionality: reset timer & pause */
    void stop() {
        pause();
    }

    /* just stop timer, no set timer. */
    void pause() {
        if (this->state == State::pause) {
            return ;
        }
        {
            std::unique_lock<std::mutex> state_lock(state_mutex);
            this->state = State::pause;
        }
        this->state_changed = true;
        cv.notify_one();
    }

    /* run from the paused time, */
    void run() {
        if (this->state == State::running) {
            return ;
        }
        {
            std::unique_lock<std::mutex> state_lock(state_mutex);
            this->state = State::running;
        }
        this->state_changed = true;
        cv.notify_one();
    }


private:
    /* basic timer data */
    Precision period;
    std::atomic<Precision> remain;

    /*for later feature: support once callback timer*/
//    std::atomic<bool> once{false};

    /* use for manage timer start & pause */
    enum class State{
        running = 1,
        pause = 2,
        shutdown = 3,
        resetting = 4,
    };
    std::condition_variable cv;
    std::mutex state_mutex;
    std::atomic<State> state{State::pause};
    std::atomic<bool> state_changed{false};

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
Timer<Precision>::Timer(Period &&period) :  period(std::forward<Period>(period)), remain(this->period) {}


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

    std::unique_lock<std::mutex> state_lock(state_mutex);
    while (this->state != State::shutdown) {

        auto start = std::chrono::system_clock::now();
        bool notified = cv.wait_for(state_lock, remain.load(), [this] () {
            return this->state_changed.load();
        });
        if (!notified) {
            this->remain = period;
            switch (this->state) {
                case State::running: {
                    /* do the callback function. */
                    std::thread t(f); t.detach();
                }break;
                case State::pause: {
                    /* pause state: do nothing, just continue loop. */
                }break;
                case State::shutdown: {
                    return;
                }break;
                default:  {
                    throw std::runtime_error("no such reachable state in waiting timeout condition. Check it!");
                }
            }
        } else {
            switch(this->state) {
                case State::running: {
//                    throw std::runtime_error("no such reachable state. Check it!");
                };break;
                case State::pause:{
                    this->remain =  this->remain.load() - std::chrono::duration_cast<Precision>(std::chrono::system_clock::now() - start);
                }; break;
                case State::resetting: {
                    this->remain = period;
                    this->state = State::running;
                }; break;
                case State::shutdown: {
                    std::cout << "shut" << std::endl;
                    return;
                }break;
                default: throw std::runtime_error("no such reachable state in awake condition. Check it!");
            }
            state_changed = false;
        }
    }
}



#endif //MIT6_824_C_TIMER_HPP



