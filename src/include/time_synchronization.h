#ifndef TIME_SYNCHRONIZATION_H
#define TIME_SYNCHRONIZATION_H

#include <chrono>
#include <thread>
#include "time_conversion.h"

namespace time_utils {

void sleep_until(const std::chrono::steady_clock::time_point& until) {
    std::this_thread::sleep_until(steady_to_system_clock(until));
}

} // namespace time_utils

#endif // TIME_SYNCHRONIZATION_H
