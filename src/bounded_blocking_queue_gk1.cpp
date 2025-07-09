/*
 Design Bounded Blocking Queue problem:
   It requires implementing a thread-safe queue with a fixed capacity. 
   The queue supports three methods: 
	 - enqueue (adds an element, blocking if the queue is full), 
	 - dequeue (removes and returns an element, blocking if the queue is empty), and 
	 - size (returns the current number of elements).
   The implementation must handle concurrent access correctly, ensuring thread safety 
   and proper blocking behavior.
*/

#include <queue>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <chrono>

class BoundedBlockingQueue {
private:
    std::queue<int> queue_;
    std::mutex mtx_;
    std::condition_variable not_empty_;
    std::condition_variable not_full_;
    size_t capacity_;

public:
    BoundedBlockingQueue(int capacity) : capacity_(capacity) {}

    void enqueue(int element) {
        std::unique_lock<std::mutex> lock(mtx_);
        not_full_.wait(lock, [this]() { return queue_.size() < capacity_; });
        queue_.push(element);
        std::printf("Enqueued %d, queue size: %zu\n", element, queue_.size());
        lock.unlock();
        not_empty_.notify_one();
    }

    int dequeue() {
        std::unique_lock<std::mutex> lock(mtx_);
        not_empty_.wait(lock, [this]() { return !queue_.empty(); });
        int element = queue_.front();
        queue_.pop();
        std::printf("Dequeued %d, queue size: %zu\n", element, queue_.size());
        lock.unlock();
        not_full_.notify_one();
        return element;
    }

    int size() {
        std::lock_guard<std::mutex> lock(mtx_);
        return queue_.size();
    }
};

// Testing function
void producer(BoundedBlockingQueue& queue, int id, int items) {
    for (int i = 0; i < items; ++i) {
        queue.enqueue(id * 100 + i);
        std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    }
}

void consumer(BoundedBlockingQueue& queue, int items) {
    for (int i = 0; i < items; ++i) {
        int value = queue.dequeue();
        std::this_thread::sleep_for(std::chrono::milliseconds(20)); // Simulate work
    }
}

int main() {
	std::printf("Compile: g++ -std=c++11 bounded_blocking_queue_gk1.cpp -o bounded_blocking_queue -pthread\n");
    // Test Case 1: Queue with capacity 3, 2 producers, 2 consumers.
    std::printf("Starting Test Case 1: Capacity 3, 2 producers, 2 consumers.\n");
    BoundedBlockingQueue queue1(3);
    std::vector<std::thread> threads1;

    threads1.emplace_back(producer, std::ref(queue1), 1, 5);
    threads1.emplace_back(producer, std::ref(queue1), 2, 5);
    threads1.emplace_back(consumer, std::ref(queue1), 5);
    threads1.emplace_back(consumer, std::ref(queue1), 5);

    for (auto& t : threads1) {
        t.join();
    }

    std::printf("Test Case 1 - Final queue size: %d\n", queue1.size());

    // Test Case 2: Queue with capacity 7, 2 producers, 3 consumers.
    std::printf("\nStarting Test Case 2: Capacity 7, 2 producers, 3 consumers.\n");
    BoundedBlockingQueue queue2(7);
    std::vector<std::thread> threads2;

    threads2.emplace_back(producer, std::ref(queue2), 3, 5); // Producer ID 3
    threads2.emplace_back(producer, std::ref(queue2), 4, 5); // Producer ID 4
    threads2.emplace_back(consumer, std::ref(queue2), 4);    // Consumer 1: 4 items
    threads2.emplace_back(consumer, std::ref(queue2), 3);    // Consumer 2: 3 items
    threads2.emplace_back(consumer, std::ref(queue2), 3);    // Consumer 3: 3 items

    for (auto& t : threads2) {
        t.join();
    }

    std::printf("Test Case 2 - Final queue size: %d\n", queue2.size());
    return 0;
}
