#ifndef TIME_CONVERSION_H
#define TIME_CONVERSION_H

#include <chrono>

namespace time_utils {

std::chrono::system_clock::time_point steady_to_system_clock(const std::chrono::steady_clock::time_point& steady_tp);

template <typename Duration>
Duration get_duration_between(const std::chrono::steady_clock::time_point& start,
                             const std::chrono::steady_clock::time_point& end) {
    return std::chrono::duration_cast<Duration>(end - start);
}

} // namespace time_utils

#endif // TIME_CONVERSION_H
