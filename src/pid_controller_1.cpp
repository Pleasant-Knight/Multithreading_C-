//pid controller with mutex from grok
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <sstream>
#include <cstdio>
#include <string>

std::atomic<bool> running(true);

struct SharedData {
    double plant_state = 1.0;    // Actual state of the system
    double sensor_value = 1.0;   // Delayed sensor reading
    std::mutex mtx;
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
	double local_plant_state = 0.0;
    while (running) {
        local_plant_state = 0.0;
        {
            std::lock_guard<std::mutex> lock(shared.mtx);
            local_plant_state = shared.plant_state;
        }
        // Simulate sensor processing delay
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        if (!running) break;
        { // this is running
            std::lock_guard<std::mutex> lock(shared.mtx);
            shared.sensor_value = local_plant_state;
            std::printf("Sensor thread %s: Updated sensor_value to %f\n", 
                        thread_id.c_str(), local_plant_state);
        }
    }
}

void controller_thread(SharedData& shared, PID& pid) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string thread_id = oss.str();
    const double setpoint = 0.0;
    const double dt = 0.01; // 10ms control loop
	double local_sensor_value = 0.0;
	double error = 0.0;
	double output = 0.0;

    while (running) {
        local_sensor_value = 0.0;
        {
            std::lock_guard<std::mutex> lock(shared.mtx);
            local_sensor_value = shared.sensor_value;
        }
        error = setpoint - local_sensor_value;
        output = pid.compute(error);
        {
            std::lock_guard<std::mutex> lock(shared.mtx);
            shared.plant_state += output * dt;
            std::printf("Controller thread %s: sensor_value = %f, output = %f, plant_state = %f\n",
                        thread_id.c_str(), local_sensor_value, output, shared.plant_state);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
	std::printf("Compile: g++ -std=c++23 -pthread <file_name.CPP> -o <app_name>\n");
    SharedData shared;
    PID pid(1.0, 0.1, 0.01, 0.01); // Example PID gains: Kp=1.0, Ki=0.1, Kd=0.01

    std::thread sensor(sensor_thread, std::ref(shared));
    std::thread controller(controller_thread, std::ref(shared), std::ref(pid));

    std::this_thread::sleep_for(std::chrono::seconds(5));
    running = false;

    sensor.join();
    controller.join();

    return 0;
} // 473 lines of asm
