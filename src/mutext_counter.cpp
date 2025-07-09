#include <cstdio>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;
int counter = 0;

void increment(int id) {
  for (int i = 0; i < 5; ++i) {
    std::lock_guard<std::mutex> lock(mtx);
    ++counter;
    std::printf("Thread %d: Counter = %d\n", id, counter);
  }
}

int main() {
  std::vector<std::thread> threads;
  std::printf("Compile: g++ -std=c++23 -pthread atomic_counter.cpp -o "
              "atomic_counter\n");
  for (int i = 1; i <= 3; ++i) {
    threads.emplace_back(increment, i);
  }
  for (auto &t : threads) {
    t.join();
  }
  std::printf("Final Counter: %d\n", counter);
}
