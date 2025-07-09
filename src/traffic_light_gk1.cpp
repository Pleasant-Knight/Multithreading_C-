//
/*
The Traffic Light Controlled Intersection:
 The problem describes a traffic light at an intersection of two roads, Road A
and Road B. Initially, Road A has a green light, allowing cars on Road A to
pass, while Road B has a red light, so cars on Road B wait. The traffic light
controller needs to:
  - Allow cars on the green road to pass without interruption.
  - Switch the green light between Road A and Road B when requested, ensuring
thread safety. The solution must implement a TrafficLight class with: void
carArrived(int carId, int roadId, int direction, function<void()> turnGreen,
function<void()> goThrough): carId: Unique car identifier (1 to 1000). roadId: 1
for Road A, 2 for Road B. direction: 1 for left, 2 for right (not directly
relevant to traffic light logic). turnGreen: Function to make the car’s road
green. goThrough: Function to let the car pass the intersection. The system must
be thread-safe, as multiple cars (threads) may call carArrived concurrently.
*/

#include <chrono>
#include <cstdio>
#include <functional>
#include <mutex>
#include <thread>

class TrafficLight {
private:
  int greenRoad; // 1 for Road A, 2 for Road B.
  std::mutex mtx;

public:
  TrafficLight() {
    greenRoad = 1; // Initially, Road A has green light.
  }

  void
  carArrived(int carId,                       // Car's unique ID.
             int roadId,                      // 1 for Road A, 2 for Road B.
             int direction,                   // 1 for left, 2 for right.
             std::function<void()> turnGreen, // Function to turn light green.
             std::function<void()> goThrough  // Function to let car pass.
  ) {
    std::lock_guard<std::mutex> lock(mtx); // Lock for thread safety.
    if (roadId != greenRoad) {
      // Car is on red-light road; switch light to green.
      turnGreen();
      greenRoad = roadId;
      std::printf("Traffic light switched to green for Road %d for car %d\n",
                  roadId, carId);
    } else {
      std::printf("Road %d already green for car %d\n", roadId, carId);
    }
    goThrough(); // Let the car pass
    std::printf("Car %d passed through intersection\n", carId);
  }
};

int main() {
  TrafficLight trafficLight;
  std::printf(
      "Compile: g++ -std=c++11 traffic_light.cpp -o traffic_light -pthread\n");

  // Mock functions to simulate LeetCode's turnGreen and goThrough.
  auto turnGreen = []() { std::printf("Traffic light turned green\n"); };

  auto goThrough = []() { std::printf("Car is passing through\n"); };

  // Simulate three cars: Road A, Road B, Road A.
  std::thread car1([&]() {
    std::printf("Car 1 arriving on Road A\n");
    trafficLight.carArrived(1, 1, 1, turnGreen, goThrough);
  });

  // Slight delay to ensure car1 goes first
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::thread car2([&]() {
    std::printf("Car 2 arriving on Road B\n");
    trafficLight.carArrived(2, 2, 1, turnGreen, goThrough);
  });

  // Delay to ensure car2 goes before car3.
  std::this_thread::sleep_for(std::chrono::milliseconds(100));

  std::thread car3([&]() {
    std::printf("Car 3 arriving on Road A\n");
    trafficLight.carArrived(3, 1, 1, turnGreen, goThrough);
  });

  // Wait for all threads to complete.
  car1.join();
  car2.join();
  car3.join();

  std::printf("All cars have passed\n");
  return 0;
}
