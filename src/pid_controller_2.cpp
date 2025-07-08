#include <thread>
#include <chrono>
#include <mutex>
#include <atomic>
#include <random>
#include <sstream>
#include <string>
#include <cstdio>

using namespace std::chrono_literals;

struct SharedSensorData {
    std::mutex mtx;
    double value = 0.0;
    std::chrono::steady_clock::time_point timestamp;
};

class PIDController {
public:
    PIDController(double kp, double ki, double kd)
        : kp_(kp), ki_(ki), kd_(kd), prev_error_(0), integral_(0) {}

    double compute(double setpoint, double measurement) {
        double error = setpoint - measurement;
        integral_ += error;
        double derivative = error - prev_error_;
        prev_error_ = error;
        return kp_ * error + ki_ * integral_ + kd_ * derivative;
    }

private:
    double kp_, ki_, kd_;
    double prev_error_;
    double integral_;
};

std::string get_thread_id_str() {
    std::stringstream ss;
    ss << std::this_thread::get_id();
    return ss.str();
}

void sensorThread(SharedSensorData& data, bool& running) {
    std::string thread_id = get_thread_id_str();
    std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<double> dist(0.0, 10.0);

    while (running) {
        std::this_thread::sleep_for(2s);  // Simulate slow sensor

        double simulated_value = dist(rng);
        {
            std::lock_guard<std::mutex> lock(data.mtx);
            data.value = simulated_value;
            data.timestamp = std::chrono::steady_clock::now();
        }

        std::printf("[SensorThread | ID: %s] New sensor value: %.2f\n",
                    thread_id.c_str(), simulated_value);
    }
}

void controlThread(SharedSensorData& data, bool& running, double setpoint) {
    std::string thread_id = get_thread_id_str();
    PIDController pid(1.0, 0.1, 0.05);

    while (running) {
        std::this_thread::sleep_for(500ms);

        double current_value;
        {
            std::lock_guard<std::mutex> lock(data.mtx);
            current_value = data.value;
        }

        double output = pid.compute(setpoint, current_value);

        std::printf("[ControlThread | ID: %s] Measured: %.2f, PID Output: %.2f\n",
                    thread_id.c_str(), current_value, output);
    }
}

int main() {
    std::printf("Compilation Command:\n");
    std::printf("g++ -std=c++20 -pthread -O2 -o pid_controller <file name.CPP>\n\n");

    SharedSensorData sensorData;
    sensorData.timestamp = std::chrono::steady_clock::now();
    bool running = true;
    double setpoint = 5.0;

    std::thread sensor(sensorThread, std::ref(sensorData), std::ref(running));
    std::thread controller(controlThread, std::ref(sensorData), std::ref(running), setpoint);

    std::this_thread::sleep_for(10s);

    running = false;
    sensor.join();
    controller.join();

    std::printf("\n[Main] Program completed. Exiting.\n");
    return 0;
} // 448 assembly lines


/*
    Executor x86-64 gcc 15.1 (C++, Editor #1)

x86-64 gcc 15.1
Compiler options for execution

Program returned: 0
Program stdout

Compilation Command:
g++ -std=c++20 -pthread -O2 -o pid_controller pid_controller.cpp

[ControlThread | ID: 134026876675648] Measured: 0.00, PID Output: 5.75
[ControlThread | ID: 134026876675648] Measured: 0.00, PID Output: 6.00
[ControlThread | ID: 134026876675648] Measured: 0.00, PID Output: 6.50
[SensorThread | ID: 134026885068352] New sensor value: 7.52
[ControlThread | ID: 134026876675648] Measured: 7.52, PID Output: -1.65
[ControlThread | ID: 134026876675648] Measured: 7.52, PID Output: -1.53
[ControlThread | ID: 134026876675648] Measured: 7.52, PID Output: -1.78
[ControlThread | ID: 134026876675648] Measured: 7.52, PID Output: -2.03
[SensorThread | ID: 134026885068352] New sensor value: 8.62
[ControlThread | ID: 134026876675648] Measured: 8.62, PID Output: -3.54
[ControlThread | ID: 134026876675648] Measured: 8.62, PID Output: -3.85
[ControlThread | ID: 134026876675648] Measured: 8.62, PID Output: -4.21
[ControlThread | ID: 134026876675648] Measured: 8.62, PID Output: -4.57
[SensorThread | ID: 134026885068352] New sensor value: 7.84
[ControlThread | ID: 134026876675648] Measured: 7.84, PID Output: -4.04
[ControlThread | ID: 134026876675648] Measured: 7.84, PID Output: -4.36
[ControlThread | ID: 134026876675648] Measured: 7.84, PID Output: -4.65
[ControlThread | ID: 134026876675648] Measured: 7.84, PID Output: -4.93
[SensorThread | ID: 134026885068352] New sensor value: 6.43
[ControlThread | ID: 134026876675648] Measured: 6.43, PID Output: -3.59
[ControlThread | ID: 134026876675648] Measured: 6.43, PID Output: -3.80
[ControlThread | ID: 134026876675648] Measured: 6.43, PID Output: -3.95
[ControlThread | ID: 134026876675648] Measured: 6.43, PID Output: -4.09
[SensorThread | ID: 134026885068352] New sensor value: 1.49
[ControlThread | ID: 134026876675648] Measured: 1.49, PID Output: 1.44

[Main] Program completed. Exiting.

*/