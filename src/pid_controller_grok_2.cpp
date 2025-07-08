// pid using atomic from grok
#include <atomic>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstdio>
#include <string>

std::atomic<bool> running(true);

struct SharedData {
    std::atomic<double> plant_state{1.0};  // Actual system state
    std::atomic<double> sensor_value{1.0}; // Delayed sensor reading
};

class PID {
public:
    PID(double Kp, double Ki, double Kd, double dt)
        : Kp(Kp), Ki(Ki), Kd(Kd), dt(dt), integral(0.0), previous_error(0.0) {}

    double compute(double error) {
        integral += error * dt;
        double derivative = (error - previous_error) / dt;
        double output = Kp * error + Ki * integral + Kd * derivative;
        previous_error = error;
        return output;
    }

private:
    double Kp, Ki, Kd, dt;
    double integral;
    double previous_error;
};

void sensor_thread(SharedData& shared) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string thread_id = oss.str();
    while (running.load(std::memory_order_relaxed)) {
        double local_plant_state = shared.plant_state.load(std::memory_order_acquire);
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); // Simulate sensor delay
        if (!running.load(std::memory_order_relaxed)) break;
        shared.sensor_value.store(local_plant_state, std::memory_order_release);
        std::printf("Sensor thread %s: Updated sensor_value to %f\n", 
                    thread_id.c_str(), local_plant_state);
    }
}

void controller_thread(SharedData& shared, PID& pid) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string thread_id = oss.str();
    const double setpoint = 0.0;
    const double dt = 0.01; // 10ms control loop

    while (running.load(std::memory_order_relaxed)) {
        double local_sensor_value = shared.sensor_value.load(std::memory_order_acquire);
        double error = setpoint - local_sensor_value;
        double output = pid.compute(error);
        double old_plant_state = shared.plant_state.load(std::memory_order_acquire);
        double new_plant_state = old_plant_state + output * dt;
        shared.plant_state.store(new_plant_state, std::memory_order_release);
        std::printf("Controller thread %s: sensor_value = %f, output = %f, plant_state = %f\n",
                    thread_id.c_str(), local_sensor_value, output, new_plant_state);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    SharedData shared;
	std::printf("Compile: g++ -std=c++23 -pthread <file_name.CPP> -o <app_name>\n");
    PID pid(1.0, 0.1, 0.01, 0.01); // Kp=1.0, Ki=0.1, Kd=0.01

    // Check if std::atomic<double> is lock-free
    if (!shared.plant_state.is_lock_free() || !shared.sensor_value.is_lock_free()) {
        std::printf("Error: std::atomic<double> is not lock-free on this platform\n");
        return 1;
    }

    std::thread sensor(sensor_thread, std::ref(shared));
    std::thread controller(controller_thread, std::ref(shared), std::ref(pid));

    std::this_thread::sleep_for(std::chrono::seconds(5));
    running.store(false, std::memory_order_release);

    sensor.join();
    controller.join();

    return 0;
} // 452 lines of asm
