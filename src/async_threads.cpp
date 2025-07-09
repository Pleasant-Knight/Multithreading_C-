#include <chrono>
#include <cstdio>
#include <future>
#include <sstream>
#include <thread>

int compute_sum(int start, int end) {
  int sum = 0;
  std::ostringstream oss;

  for (int i = start; i <= end; ++i) {
    sum += i;
  }
  oss << std::this_thread::get_id();
  std::printf("Computed sum from %d to %d in thread ID: %s\n", start, end,
              oss.str().c_str());
  return sum;
}

void process_data(int id) {
  std::ostringstream oss;
  std::this_thread::sleep_for(std::chrono::milliseconds(500));
  oss << std::this_thread::get_id();
  std::printf("Processed data for ID %d in thread ID: %s\n", id,
              oss.str().c_str());
}

int main() {
  std::ostringstream oss;
  std::printf(
      "Compile: g++ -std=c++23 -pthread <file_name> -o <output_name>\n");
  std::future<int> result1 =
      std::async(std::launch::async, compute_sum, 1, 1000);
  std::future<void> result2 = std::async(std::launch::async, process_data, 42);
  oss << std::this_thread::get_id();
  std::printf("Main thread ID: %s\n", oss.str().c_str());

  try {
    int sum = result1.get();
    std::printf("Sum result: %d\n", sum);
    result2.get();
  } catch (const std::exception &e) {
    std::printf("Exception: %s\n", e.what());
  }
}
