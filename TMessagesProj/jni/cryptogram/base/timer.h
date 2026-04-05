#pragma once
#include <chrono>

namespace base {
class Timer {
public:
    void start(int msec) {}
    void stop() {}
    bool isActive() const { return false; }
};
}
