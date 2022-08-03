#ifndef _TimerClock_hpp_
#define _TimerClock_hpp_

#include <iostream>
#include <chrono>

using namespace std;
using namespace std::chrono;

class TimerClock {
public:
    TimerClock() {
        update();
    }

    ~TimerClock() {}

    void update()
    {
        _start = high_resolution_clock::now();
    }

    //获取秒
    double get_timer_second() {
        return get_timer_microsec() * 0.000001;
    }

    //获取毫秒
    double get_timer_millisec() {
        return get_timer_microsec() * 0.001;
    }

    //获取微妙
    long long get_timer_microsec() {
        //当前时钟减去开始时钟的count
        return duration_cast<microseconds>(high_resolution_clock::now() - _start).count();
    }

private:
    time_point<high_resolution_clock> _start;
};

#endif
