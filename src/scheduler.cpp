#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <functional>
#include <string>
#include "./include/time_formatter.h"
#include <stdio.h>

// Task structure
struct Task {
    int id;
    int priority; // Higher value = higher priority
    std::chrono::milliseconds period;
    std::chrono::milliseconds execution_time;
    std::chrono::system_clock::time_point next_deadline;
    std::function<void()> work;

    Task(int _id, int _priority, int period_ms, int exec_time_ms, std::function<void(int)> _work)
        : id(_id), priority(_priority), period(std::chrono::milliseconds(period_ms)),
          execution_time(std::chrono::milliseconds(exec_time_ms)), next_deadline(std::chrono::system_clock::now()),
          work([_id, this, _work]() { _work(id); })
    {
        std::printf("Created task %d: Priority=%d, Period=%ldms, ExecTime=%ldms at %s\n",
                    id, priority, period.count(), execution_time.count(),
                    time_utils::formatTime(std::chrono::system_clock::now()).c_str());
    }
};

// Comparator for priority queue (Earliest Deadline First)
struct TaskComparator {
    bool operator()(const Task* t1, const Task* t2) const {
        if (t1->next_deadline == t2->next_deadline)
            return t1->priority < t2->priority; // Same deadline, higher priority first
        return t1->next_deadline > t2->next_deadline; // Earlier deadline first
    }
};

class Scheduler {
private:
    std::vector<Task> tasks;
    std::priority_queue<Task*, std::vector<Task*>, TaskComparator> ready_queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool running;
    std::thread scheduler_thread;

    void schedulerLoop() {
        while (true) {
            std::unique_lock<std::mutex> lock(mtx);
            if (!running) break;

            auto now = std::chrono::system_clock::now();
            // Check for tasks ready to run
            for (auto& task : tasks) {
                if (task.next_deadline <= now) {
                    std::printf("Task %d ready at %s\n", task.id, time_utils::formatTime(now).c_str());
                    ready_queue.push(&task);
                    task.next_deadline += task.period; // Schedule next instance
                }
            }

            // Execute highest-priority task
            if (!ready_queue.empty()) {
                Task* task = ready_queue.top();
                ready_queue.pop();
                lock.unlock();

                auto start_time = std::chrono::system_clock::now();
                std::printf("Executing task %d at %s\n", task->id, time_utils::formatTime(start_time).c_str());
                task->work();
                std::this_thread::sleep_for(task->execution_time); // Simulate work1
                auto end_time = std::chrono::system_clock::now();

                if (end_time > task->next_deadline) {
                    std::printf("Task %d missed deadline at %s\n", task->id, time_utils::formatTime(end_time).c_str());
                } else {
                    std::printf("Task %d completed at %s\n", task->id, time_utils::formatTime(end_time).c_str());
                }
            } else {
                // Wait until the next task is ready or new tasks are added
                if (!tasks.empty()) {
                    auto next_deadline = tasks[0].next_deadline;
                    for (const auto& task : tasks) {
                        if (task.next_deadline < next_deadline)
                            next_deadline = task.next_deadline;
                    }
                    cv.wait_until(lock, next_deadline, [this]() { return !running || !ready_queue.empty(); });
                } else {
                    cv.wait(lock, [this]() { return !running || !tasks.empty(); });
                }
            }
        }
    }

public:
    Scheduler() : running(false) {}

    void addTask(int id, int priority, int period_ms, int exec_time_ms, std::function<void(int)> work) {
        std::lock_guard<std::mutex> lock(mtx);
        tasks.emplace_back(id, priority, period_ms, exec_time_ms, work);
        cv.notify_one();
        std::printf("Added task %d to scheduler at %s\n", id, time_utils::formatTime(std::chrono::system_clock::now()).c_str());
    }

    void start() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!running) {
            running = true;
            scheduler_thread = std::thread(&Scheduler::schedulerLoop, this);
            std::printf("Scheduler started at %s\n", time_utils::formatTime(std::chrono::system_clock::now()).c_str());
        }
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            running = false;
            cv.notify_one();
        }
        if (scheduler_thread.joinable()) {
            scheduler_thread.join();
            std::printf("Scheduler stopped at %s\n", time_utils::formatTime(std::chrono::system_clock::now()).c_str());
        }
    }

    ~Scheduler() {
        stop();
    }
};

// Sample task functions
void sensorReading(int id) {
    std::printf("Task %d: Reading sensor data at %s\n", id, time_utils::formatTime(std::chrono::system_clock::now()).c_str());
}

void controlLoop(int id) {
    std::printf("Task %d: Executing control loop at %s\n", id, time_utils::formatTime(std::chrono::system_clock::now()).c_str());
}

int main() {
    Scheduler scheduler;

    // Add tasks: (id, priority, period_ms, exec_time_ms, work_function)
    scheduler.addTask(1, 2, 1000, 200, sensorReading); // Sensor reading every 1s, takes 200ms
    scheduler.addTask(2, 1, 2000, 300, controlLoop);   // Control loop every 2s, takes 300ms
    scheduler.addTask(3, 3, 500, 100, sensorReading);  // High-priority sensor every 500ms, takes 100ms

    scheduler.start();

    // Run for 10 seconds
    std::this_thread::sleep_for(std::chrono::seconds(10));

    scheduler.stop();

    return 0;
}
