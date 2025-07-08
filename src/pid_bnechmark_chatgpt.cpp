// pid_bench_cpp20.cpp
#include <benchmark/benchmark.h>
#include <thread>
#include <mutex>
#include <atomic>
#include <cmath>

struct SharedData {
    std::mutex mtx;
    double value = 0.0;
};

class PID {
public:
    PID(double kp, double ki, double kd) : kp(kp), ki(ki), kd(kd), prev_error(0), integral(0) {}

    double compute(double setpoint, double measurement) {
        double error = setpoint - measurement;
        integral += error;
        double derivative = error - prev_error;
        prev_error = error;
        return kp * error + ki * integral + kd * derivative;
    }

private:
    double kp, ki, kd;
    double prev_error, integral;
};

static void BM_CPP20_PID_Multithread(benchmark::State& state) {
    const int iterations = static_cast<int>(state.range(0));

    for (auto _ : state) {
        SharedData shared;
        std::atomic<bool> running{true};
        PID pid(1.0, 0.1, 0.05);

        std::thread sensor([&]() {
            for (int i = 0; i < iterations && running; ++i) {
                std::lock_guard<std::mutex> lock(shared.mtx);
                shared.value = i * 0.001;
            }
        });

        std::thread controller([&]() {
            for (int i = 0; i < iterations && running; ++i) {
                double val;
                {
                    std::lock_guard<std::mutex> lock(shared.mtx);
                    val = shared.value;
                }
                pid.compute(5.0, val);
            }
        });

        sensor.join();
        controller.join();
        running = false;
    }
}

BENCHMARK(BM_CPP20_PID_Multithread)->Arg(1000000);
BENCHMARK_MAIN();
//g++ -std=c++20 -O2 -pthread pid_bnechmark_chatgpt.cpp -lbenchmark -o cpp_bench
