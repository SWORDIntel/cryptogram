#pragma once
#include <chrono>

#ifndef BASE_TIMER_DEFINED
#define BASE_TIMER_DEFINED
namespace base {
class Timer {
public:
    void start(int msec) {}
    void stop() {}
    bool isActive() const { return false; }
};
}
#endif
