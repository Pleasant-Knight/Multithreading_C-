#include <mutex>
#include <thread>
#include <vector>
#include <chrono>

constexpr int NUM_PHILOSOPHERS = 5;

class DiningPhilosophers {
private:
    std::mutex forks[NUM_PHILOSOPHERS]; // Mutexes for each fork
    // Using mutex for printing to avoid garbled output from multiple threads.
    std::mutex print_mutex; // For thread-safe printing.

public:
    DiningPhilosophers() {     }

    void wantsToEat(int philosopher,
                    std::function<void()> pickLeftFork,
                    std::function<void()> pickRightFork,
                    std::function<void()> eat,
                    std::function<void()> putLeftFork,
                    std::function<void()> putRightFork) {
        int left = philosopher;
        int right = (philosopher + 1) % NUM_PHILOSOPHERS;

        // To avoid deadlock, philosopher 4 picks right fork first
        if (philosopher == (NUM_PHILOSOPHERS - 1)) {
            std::lock_guard<std::mutex> right_lock(forks[right]);
            std::lock_guard<std::mutex> left_lock(forks[left]);
            // Print statement to indicate which philosopher is eating
            {
                std::lock_guard<std::mutex> print_lock(print_mutex);
                std::printf("Philosopher %d is eating\n", philosopher);
            }
            
            pickRightFork();
            pickLeftFork();
            eat();
            putLeftFork();
            putRightFork();
        } else {
            std::lock_guard<std::mutex> left_lock(forks[left]);
            std::lock_guard<std::mutex> right_lock(forks[right]);
            
            {
                std::lock_guard<std::mutex> print_lock(print_mutex);
                std::printf("Philosopher %d is eating\n", philosopher);
            }
            
            pickLeftFork();
            pickRightFork();
            eat();
            putLeftFork();
            putRightFork();
        }
    }
};

// Test functions to simulate philosopher actions
void pickLeftFork() { std::printf("Picked left fork\n"); }
void pickRightFork() { std::printf("Picked right fork\n"); }
void eat() { 
    std::printf("Eating\n"); 
    std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate eating time
}
void putLeftFork() { std::printf("Put down left fork\n"); }
void putRightFork() { std::printf("Put down right fork\n"); }

// Main function to test the DiningPhilosophers class
int main() {
    DiningPhilosophers dp;
    std::vector<std::thread> threads;

    // Create 5 philosopher threads, each trying to eat 3 times
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([i, &dp]() {
            for (int j = 0; j < 3; ++j) {
                dp.wantsToEat(i, pickLeftFork, pickRightFork, eat, putLeftFork, putRightFork);
                // Simulate thinking time
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }

    // Join all threads
    for (auto& t : threads) {
        t.join();
    }

    std::printf("All philosophers have finished eating.\n");
    return 0;
}