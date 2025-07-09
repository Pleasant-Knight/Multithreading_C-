#include <atomic>
#include <cstdint>
#include <cstdio>
#include <thread>
#include <vector>

std::atomic<uint64_t> counter(0);

void increment(int id) {
  for (int i = 0; i < 5; ++i) {
    uint64_t new_value = counter.fetch_add(1, std::memory_order_relaxed) + 1;
    std::printf("Thread %d: Counter = %lu\n", id, new_value);
  }
}

int main() {
  std::vector<std::thread> threads;
  std::printf(
      "Compile: g++ -std=c++23 -pthread <file_name> -o <output_name>\n");
  for (int i = 1; i <= 3; ++i) {
    threads.emplace_back(increment, i);
  }
  for (auto &t : threads) {
    t.join();
  }
  std::printf("Final Counter: %lu\n", counter.load());
}
