#include <print>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> q;

void producer() {
    std::lock_guard<std::mutex> lock(mtx);
    q.push(42);
    cv.notify_one();
}

void consumer() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [] { return !q.empty(); });
    std::println("Consumed: {}", q.front());
    q.pop();
}

int main() {
	std::printf("Compile: g++ -std=c++23 -pthread <file_name> -o <output_name>\n");
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
}
