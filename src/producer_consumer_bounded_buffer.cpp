// Producer-Consumer Bounded Buffer:
#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <deque>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>

// Shared Globals
constexpr size_t BUFFER_SIZE = 10;
constexpr int MAX_ITEMS = 50;
std::atomic<bool> running(true);

/*
 * std::deque (double-ended queue) is an indexed sequence container that allows
 * fast insertion and deletion at both its beginning and its end. In addition,
 * insertion and deletion at either end of a deque never invalidates pointers or
 * references to the rest of the elements.
 */

struct SharedBuffer {
  std::mutex mtx;
  std::deque<int> buffer;
  std::condition_variable not_empty;
  std::condition_variable not_full;
};

void producer(SharedBuffer &shared, int &prod_count) {
  std::ostringstream oss;
  oss << std::this_thread::get_id();
  auto th_id = oss.str().c_str();
  int item = 1;

  while (item <= MAX_ITEMS) {
    std::unique_lock<std::mutex> lock(shared.mtx);
    shared.not_full.wait(
        lock, [&]() { return shared.buffer.size() < BUFFER_SIZE || !running; });
    if (!running && item > MAX_ITEMS)
      break;

    shared.buffer.push_back(item);
    std::printf("Prod thread %s: item: %d, buf_size: %zu\n", th_id, item,
                shared.buffer.size());
    ++item;
    ++prod_count;

    lock.unlock();
    shared.not_empty.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  return;
}

void consumer(SharedBuffer &shared, int &cons_count) {
  std::ostringstream oss;
  oss << std::this_thread::get_id();
  auto th_id = oss.str().c_str();

  while (cons_count < MAX_ITEMS) {
    std::unique_lock<std::mutex> lock(shared.mtx);
    shared.not_empty.wait(lock,
                          [&]() { return !shared.buffer.empty() || !running; });
    if (!running && shared.buffer.empty())
      break;
    auto item = shared.buffer.front();
    shared.buffer.pop_front();
    std::printf("Cons thread %s: item: %d, buf_size: %zu\n", th_id, item,
                shared.buffer.size());
    ++cons_count;
    lock.unlock();
    shared.not_full.notify_one();
    std::this_thread::sleep_for(std::chrono::milliseconds(140));
  }

  return;
}

int main() {
  std::printf(
      "Compile: g++ -std=c++23 -pthread <file_name.CPP> -o <app_name>\n");
  SharedBuffer shared;
  int produced_count = 0;
  int consumed_count = 0;

  std::thread prod(producer, std::ref(shared), std::ref(produced_count));
  std::thread cons(consumer, std::ref(shared), std::ref(consumed_count));

  while (produced_count < MAX_ITEMS || consumed_count < MAX_ITEMS) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {
      std::lock_guard<std::mutex> lock(shared.mtx);
      produced_count = shared.buffer.size() + consumed_count;
    }
  }
  running = false;
  shared.not_empty.notify_one();
  shared.not_full.notify_one();

  prod.join();
  cons.join();

  std::printf("Final: Produced %d items, Consumed %d items\n", produced_count,
              consumed_count);

  return 0;
} //

/*

#include <atomic>
#include <condition_variable>
#include <cstdio>
#include <mutex>
#include <sstream>
#include <string>
#include <thread>
#include <vector>

constexpr size_t BUFFER_SIZE = 10;
constexpr int MAX_ITEMS = 50;

std::atomic<bool> running(true);

struct SharedBuffer {
    std::vector<int> buffer{BUFFER_SIZE, 0}; // Fixed-size buffer
    size_t head = 0; // Consumer removes from head
    size_t tail = 0; // Producer adds to tail
    size_t count = 0; // Number of items in buffer
    std::mutex mtx;
    std::condition_variable not_full;
    std::condition_variable not_empty;
};

void producer(SharedBuffer& shared, int& produced_count) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string thread_id = oss.str();
    int item = 1;

    while (item <= MAX_ITEMS) {
        std::unique_lock<std::mutex> lock(shared.mtx);
        shared.not_full.wait(lock, [&] { return shared.count < BUFFER_SIZE ||
!running; }); if (!running && item > MAX_ITEMS) break;

        shared.buffer[shared.tail] = item;
        shared.tail = (shared.tail + 1) % BUFFER_SIZE;
        ++shared.count;
        std::printf("Producer thread %s: Produced item %d, buffer size = %zu\n",
                    thread_id.c_str(), item, shared.count);
        ++item;
        ++produced_count;

        lock.unlock();
        shared.not_empty.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate
work
    }
}

void consumer(SharedBuffer& shared, int& consumed_count) {
    std::ostringstream oss;
    oss << std::this_thread::get_id();
    std::string thread_id = oss.str();

    while (consumed_count < MAX_ITEMS) {
        std::unique_lock<std::mutex> lock(shared.mtx);
        shared.not_empty.wait(lock, [&] { return shared.count > 0 || !running;
}); if (!running && shared.count == 0) break;

        int item = shared.buffer[shared.head];
        shared.head = (shared.head + 1) % BUFFER_SIZE;
        --shared.count;
        std::printf("Consumer thread %s: Consumed item %d, buffer size = %zu\n",
                    thread_id.c_str(), item, shared.count);
        ++consumed_count;

        lock.unlock();
        shared.not_full.notify_one();
        std::this_thread::sleep_for(std::chrono::milliseconds(150)); // Simulate
processing
    }
}

int main() {
    SharedBuffer shared;
    int produced_count = 0;
    int consumed_count = 0;

    std::thread prod(producer, std::ref(shared), std::ref(produced_count));
    std::thread cons(consumer, std::ref(shared), std::ref(consumed_count));

    while (produced_count < MAX_ITEMS || consumed_count < MAX_ITEMS) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lock(shared.mtx);
            produced_count = shared.count + consumed_count;
        }
    }
    running = false;
    shared.not_empty.notify_one();
    shared.not_full.notify_one();

    prod.join();
    cons.join();

    std::printf("Final: Produced %d items, Consumed %d items\n", produced_count,
consumed_count); return 0;
}

*/