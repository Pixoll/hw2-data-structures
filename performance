#pragma once

#include <chrono>
#include <iostream>

using namespace std;
using namespace chrono::_V2;

/**
 * Track time in small units of time
 */
class performance {
  public:
    typedef chrono::nanoseconds nanoseconds;
    typedef chrono::microseconds microseconds;
    typedef chrono::milliseconds milliseconds;

  private:
    typedef system_clock::time_point time_point;

    time_point _start;

  public:
    performance() {}

    ~performance() {}

    /** Start measuring */
    inline void start() {
        this->_start = system_clock::now();
    }

    /** Get elapsed time in the desired unit (default: ns) */
    template <typename D = nanoseconds> inline int64_t end() {
        return chrono::duration_cast<D>(system_clock::now() - this->_start).count();
    }
};
