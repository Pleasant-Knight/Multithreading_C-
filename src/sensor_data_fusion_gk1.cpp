//Sensor_data Fusion
#include <thread>
#include <vector>
#include <mutex>
#include <atomic>
#include <chrono>
#include <random>
#include <cstdio>

using std::chrono::steady_clock;

// Step 1: Define the sensor data structure
struct SensorData {
    double value; // Sensor reading (e.g., position, velocity)
    steady_clock::time_point timestamp;
};

// Step 2: Define the fused state estimate
struct StateEstimate {
    double fused_value; // Combined sensor reading
    steady_clock::time_point last_updated;
};

// Step 3: Sensor fusion class to manage thread-safe data fusion
class SensorFusion {
private:
    StateEstimate state; // Shared state estimate
    std::mutex state_mutex; // Mutex for thread-safe access
    std::atomic<bool> running{true}; // Atomic flag to control thread execution
    std::vector<SensorData> sensor_readings; // Buffer for sensor data
    std::mutex readings_mutex; // Mutex for sensor readings buffer

    // Simulate sensor reading with random data for demonstration
    SensorData read_sensor(const char* sensor_name, int sensor_id) {
        static std::random_device rd;
        static std::mt19937 gen(rd());
        static std::uniform_real_distribution<> dis(0.0, 100.0);
        
        SensorData data;
        data.value = dis(gen);
        data.timestamp = steady_clock::now();
        std::printf("[DEBUG] Sensor %s (ID: %d) read value: %.2f at time %ld\n",
                    sensor_name, sensor_id, data.value,
                    data.timestamp.time_since_epoch().count());
        return data;
    }

    // Fuse sensor data (simple averaging for demonstration)
    void fuse_data() {
        std::lock_guard<std::mutex> lock(readings_mutex);
        if (sensor_readings.empty()) {
            std::printf("[DEBUG] No sensor data to fuse\n");
            return;
        }

        double sum = 0.0;
        for (const auto& data : sensor_readings) {
            sum += data.value;
        }
        double average = sum / sensor_readings.size();

        // Update shared state
        {
            std::lock_guard<std::mutex> state_lock(state_mutex);
            state.fused_value = average;
            state.last_updated = steady_clock::now();
            std::printf("[DEBUG] Fused state updated: value = %.2f at time %ld\n",
                        state.fused_value, state.last_updated.time_since_epoch().count());
        }
        sensor_readings.clear(); // Clear readings after fusion
    }

public:
    SensorFusion() : state{0.0, steady_clock::now()} {}

    // Sensor thread function.
    void sensor_thread(const char* sensor_name, int sensor_id) {
        while (running) {
            // Simulate sensor reading.
            SensorData data = read_sensor(sensor_name, sensor_id);

            { // Store reading in buffer
                std::lock_guard<std::mutex> lock(readings_mutex);
                sensor_readings.push_back(data);
                std::printf("[DEBUG] Sensor %s (ID: %d) stored reading, buffer size: %zu\n",
                            sensor_name, sensor_id, sensor_readings.size());
            }

            // Simulate sensor processing delay.
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::printf("[DEBUG] Sensor %s (ID: %d) thread stopped\n", sensor_name, sensor_id);
    }

    // Fusion thread function.
    void fusion_thread() {
        while (running) {
            fuse_data();
            std::this_thread::sleep_for(std::chrono::milliseconds(200)); // Fuse every 200ms
        }
        std::printf("[DEBUG] Fusion thread stopped\n");
    }

    // Stop all threads
    void stop() {
        running = false;
        std::printf("[DEBUG] Stopping all threads\n");
    }

    // Get current state (thread-safe)
    StateEstimate get_state()  {
        std::lock_guard<std::mutex> lock(state_mutex);
        return state;
    }
};

// Step 4: Main function to demonstrate the system.
int main() {
    std::printf("[DEBUG] Starting sensor fusion system\n");
    std::printf("compile: g++ -std=c++23 <file_name.cpp> -o sensor_fusion -pthread\n");
    
    SensorFusion fusion;
    std::vector<std::jthread> threads;

    // Step 5: Start sensor threads (IMU, Camera, Lidar).
    threads.emplace_back([&fusion](int id) { fusion.sensor_thread("IMU", id); }, 1);
    threads.emplace_back([&fusion](int id) { fusion.sensor_thread("Camera", id); }, 2);
    threads.emplace_back([&fusion](int id) { fusion.sensor_thread("Lidar", id); }, 3);
    
    // Start fusion thread.
    threads.emplace_back([&fusion]() { fusion.fusion_thread(); });

    // Step 6: Run for 2 seconds.
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // Step 7: Stop the system.
    fusion.stop();

    // Step 8: Wait for threads to finish.
    for (auto& thread : threads) {
        thread.join();
    }

    // Step 9: Print final state
    auto final_state = fusion.get_state();
    std::printf("[DEBUG] Final fused state: value = %.2f at time %ld\n",
                final_state.fused_value, final_state.last_updated.time_since_epoch().count());

    std::printf("[DEBUG] System shutdown\n");
    return 0;
}
