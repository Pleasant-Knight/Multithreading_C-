// print_first.cpp
/*Suppose we have a class:

public class Foo {
  public void first() { print("first"); }
  public void second() { print("second"); }
  public void third() { print("third"); }
}
The same instance of Foo will be passed to three different threads. Thread A
will call first(), thread B will call second(), and thread C will call third().
Design a mechanism and modify the program to ensure that second() is executed
after first(), and third() is executed after second().

Note:

We do not know how the threads will be scheduled in the operating system, even
though the numbers in the input seem to imply the ordering. The input format you
see is mainly to ensure our tests' comprehensiveness.



Example 1:

Input: nums = [1,2,3]
Output: "firstsecondthird"
Explanation: There are three threads being fired asynchronously. The input
[1,2,3] means thread A calls first(), thread B calls second(), and thread C
calls third(). "firstsecondthird" is the correct output. Example 2:

Input: nums = [1,3,2]
Output: "firstsecondthird"
Explanation: The input [1,3,2] means thread A calls first(), thread B calls
third(), and thread C calls second(). "firstsecondthird" is the correct output.
*/

#include <algorithm>
#include <condition_variable>
#include <cstdio>
#include <functional>
#include <mutex>
#include <random>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

class Foo {
private:
  std::mutex mtx;
  std::condition_variable cv;
  int step;

public:
  Foo() : step(0) {}

  void first(std::function<void()> printFirst) {
    std::unique_lock<std::mutex> lock(mtx);
    printFirst();
    step = 1;
    cv.notify_all();
  }

  void second(std::function<void()> printSecond) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return step >= 1; });
    printSecond();
    step = 2;
    cv.notify_all();
  }

  void third(std::function<void()> printThird) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, [this]() { return step >= 2; });
    printThird();
  }
};

// Simulated print functions using std::printf
void printFirst() { std::printf("first"); }

void printSecond() { std::printf("second"); }

void printThird() { std::printf("third"); }

int main() {
  Foo foo;
  std::printf(
      "Compile: g++ -std=c++23 -pthread <file_name.CPP> -o <app_name>\n");

  // Vector of pair<name, thread creation function>
  std::vector<std::pair<std::string, std::function<std::thread()>>>
      thread_creators = {
          {"first",
           [&]() { return std::thread([&]() { foo.first(printFirst); }); }},
          {"second",
           [&]() { return std::thread([&]() { foo.second(printSecond); }); }},
          {"third",
           [&]() { return std::thread([&]() { foo.third(printThird); }); }}};

  // Randomly shuffle the order of threads
  std::random_device rd;
  std::mt19937 g(rd());
  std::shuffle(thread_creators.begin(), thread_creators.end(), g);

  // Print the randomized order
  std::printf("Shuffled thread start order: ");
  for (const auto &pair : thread_creators) {
    std::printf("%s ", pair.first.c_str());
  }
  std::printf("\n");

  // Create and start threads in shuffled order
  std::vector<std::thread> threads;
  for (auto &pair : thread_creators) {
    threads.emplace_back(pair.second());
  }

  // Join all threads
  for (auto &t : threads) {
    t.join();
  }

  std::printf("\n"); // Add newline at the end
  return 0;
}
