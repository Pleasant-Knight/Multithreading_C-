#include <thread>
#include <mutex>
#include <condition_variable>
#include <cstdio>

class FooBar {
private:
    int n;
    std::mutex mtx;
    std::condition_variable cv;
    bool foo_turn = true;

public:
    FooBar(int count) : n(count) {}

    void foo() {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return foo_turn; });
            std::printf("foo");
            foo_turn = false;
            cv.notify_all();
        }
    }

    void bar() {
        for (int i = 0; i < n; ++i) {
            std::unique_lock<std::mutex> lock(mtx);
            cv.wait(lock, [&]() { return !foo_turn; });
            std::printf("bar");
            foo_turn = true;
            cv.notify_all();
        }
    }
};

int main() {
	std::printf("Compile: g++ -std=c++23 -pthread <file_name.CPP> -o <app_name>\n");
    int n = 4;  // Change this value to test other inputs
    FooBar foobar(n);

    std::thread t1([&]() { foobar.foo(); });
    std::thread t2([&]() { foobar.bar(); });

    t1.join();
    t2.join();

    std::printf("\n");  // Add newline at the end
    return 0;
}
