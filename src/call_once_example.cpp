#include <cstdio>
#include <mutex>
#include <thread>

std::once_flag flag;

void init_once(int ID) {
  std::call_once(flag, [&] { std::println("Initialized once: {}", ID); });
}

int main() {
  std::printf(
      "Compile: g++ -std=c++23 -pthread <fill_name.CPP> -o <app_name>\n");
  std::thread t1(init_once, 1);
  std::thread t2(init_once, 2);
  t1.join();
  t2.join();
}
