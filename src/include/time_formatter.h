#ifndef TIME_FORMATTER_H
#define TIME_FORMATTER_H

#include <chrono>
#include <string>
#include <ctime>
#include <thread>
#include <iomanip>
#include <sstream>

namespace time_utils {

// Format a steady_clock time point to a human-readable string (YYYY-MM-DD HH:MM:SS)
std::string format_steady_clock_to_string(const std::chrono::steady_clock::time_point& tp) {
    // Convert steady_clock time point to system_clock time point (approximation)
    auto now_system = std::chrono::system_clock::now();
    auto now_steady = std::chrono::steady_clock::now();
    auto steady_duration = tp - now_steady;
    auto system_tp = now_system + steady_duration;

    // Convert to time_t for formatting
    auto time_t_val = std::chrono::system_clock::to_time_t(system_tp);

    // Format into string
    char buffer[64];
    if (std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&time_t_val))) {
        return std::string(buffer);
    } else {
        std::fprintf(stderr, "Error formatting time\n");
        return std::string("Invalid time");
    }
}

std::string formatTime(std::chrono::system_clock::time_point time) {
    auto tt = std::chrono::system_clock::to_time_t(time);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(time.time_since_epoch()) % 1000;
    std::stringstream ss;
    ss << std::put_time(std::localtime(&tt), "%Y-%m-%d %H:%M:%S");
    ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return ss.str();
}

std::string format_duration(const std::chrono::nanoseconds& duration) {
    using namespace std::chrono;
    auto hrs = duration_cast<hours>(duration).count();
    auto mins = duration_cast<minutes>(duration % hours(1)).count();
    auto secs = duration_cast<seconds>(duration % minutes(1)).count();
    auto ms = duration_cast<milliseconds>(duration % seconds(1)).count();

    std::stringstream ss;
    ss << std::setfill('0');
    ss << hrs << ":" << std::setw(2) << mins << ":" << std::setw(2) << secs;
    ss << "." << std::setw(3) << ms;
    return ss.str();
}

// Format a duration to a human-readable string (e.g., "2h 15m 30s")
/*std::string format_duration(const std::chrono::nanoseconds& duration) {
    using namespace std::chrono;
    char buffer[128];
    auto hours = duration_cast<hours>(duration).count();
    auto minutes = duration_cast<minutes>(duration % hours(1)).count();
    auto seconds = duration_cast<seconds>(duration % minutes(1)).count();
    auto milliseconds = duration_cast<milliseconds>(duration % seconds(1)).count();

    int written = 0;
    buffer[0] = '\0';
    if (hours > 0) {
        written += std::snprintf(buffer + written, sizeof(buffer) - written, "%lldh ", hours);
    }
    if (minutes > 0 || hours > 0) {
        written += std::snprintf(buffer + written, sizeof(buffer) - written, "%lldm ", minutes);
    }
    if (seconds > 0 || minutes > 0 || hours > 0) {
        written += std::snprintf(buffer + written, sizeof(buffer) - written, "%llds", seconds);
    } else {
        written += std::snprintf(buffer + written, sizeof(buffer) - written, "%lldms", milliseconds);
    }

    if (written >= 0 && written < static_cast<int>(sizeof(buffer))) {
        return std::string(buffer);
    } else {
        std::fprintf(stderr, "Error formatting duration\n");
        return std::string("Invalid duration");
    }
}*/

// Convert steady_clock time point to system_clock time point
std::chrono::system_clock::time_point steady_to_system_clock(const std::chrono::steady_clock::time_point& steady_tp) {
    auto now_system = std::chrono::system_clock::now();
    auto now_steady = std::chrono::steady_clock::now();
    auto steady_duration = steady_tp - now_steady;
    return now_system + steady_duration;
}

// Get duration between two steady_clock time points in a specified duration type
template <typename Duration>
Duration get_duration_between(const std::chrono::steady_clock::time_point& start,
                             const std::chrono::steady_clock::time_point& end) {
    return std::chrono::duration_cast<Duration>(end - start);
}

// Get current time with a custom format string
std::string get_current_time_custom_format(const char* format) {
    auto now = std::chrono::system_clock::now();
    auto time_t_val = std::chrono::system_clock::to_time_t(now);
    char buffer[64];
    if (std::strftime(buffer, sizeof(buffer), format, std::localtime(&time_t_val))) {
        return std::string(buffer);
    } else {
        std::fprintf(stderr, "Error formatting current time\n");
        return std::string("Invalid time");
    }
}

// Sleep until a specified steady_clock time point
void sleep_until(const std::chrono::steady_clock::time_point& until) {
    std::this_thread::sleep_until(steady_to_system_clock(until));
}

} // namespace time_utils

#endif // TIME_FORMATTER_H
