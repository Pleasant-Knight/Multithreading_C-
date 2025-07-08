#include <print>
#include <mutex>
#include <thread>

std::mutex mtx;
int counter = 0;

void increment(int thread_id) {
    for (int i = 0; i < 1000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
        std::println("Thread {}: Counter = {}", thread_id, counter);
    }
}

int main() {
	std::printf("Compile: g++ -std=c++23 -pthread <file_name> -o <output_name>\n");
    std::thread t1(increment, 1);
    std::thread t2(increment, 2);
    t1.join();
    t2.join();
    std::println("Final Counter: {}", counter); // Expected: 2000
}
