#include <vector>
#include <algorithm>
/*
First, compute the gaps between the events:
  Gap before the first event: startTime[0].
  Gaps between events: startTime[i] - endTime[i - 1].
  Gap after the last event: eventTime - endTime[n - 1].
  We store these gaps in a vector gap of size n + 1.
    Then, apply a sliding window of size k + 1 to find the maximum sum of 
	any k + 1 contiguous gaps.
*/

int maxFreeTime(int eventTime, int k, std::vector<int>& startTime, std::vector<int>& endTime) {
    int n = startTime.size();
    
    // If no meetings, return entire event time.
    if (n == 0) return eventTime;
    
    // If k >= n, we can move all meetings to one side, freeing up eventTime.
    if (k >= n) return eventTime;

    // Step 1: Compute gaps.
    std::vector<long long> gap(n + 1);
    gap[0] = startTime[0]; // Gap before first event.
    for (int i = 1; i < n; ++i) {
        gap[i] = startTime[i] - endTime[i - 1]; // Gaps between events.
    }
    gap[n] = eventTime - endTime[n - 1]; // Gap after last event.

    // Step 2: Sliding window to find max sum of k + 1 contiguous gaps.
    long long maxSum = 0;
    long long currentSum = 0;
    int windowSize = k + 1;

    // Initial window sum
    for (int i = 0; i < windowSize && i <= n; ++i) {
        currentSum += gap[i];
    }
    maxSum = currentSum;

    // Slide the window.
    for (int i = windowSize; i <= n; ++i) {
        currentSum += gap[i] - gap[i - windowSize];
        maxSum = std::max(maxSum, currentSum);
    }

    return static_cast<int>(maxSum);
}
int main() {
	std::printf("Compile: g++ -std=c++11 -o max_free_time max_free_time.cpp \n");
	std::printf("Example 1\n");
    // Example 1:
    int eventTime1 = 5, k1 = 1;
    std::vector<int> startTime1 = {1, 3};
    std::vector<int> endTime1 = {2, 5};
    std::printf("Example 1 Output: %d\n", maxFreeTime(eventTime1, k1, startTime1, endTime1));

	std::printf("Example 2\n");
	// Example 2:
    int eventTime2 = 10, k2 = 1;
    std::vector<int> startTime2 = {0, 2, 9};
    std::vector<int> endTime2 = {1, 4, 10};
    std::printf("Example 2 Output: %d\n", maxFreeTime(eventTime2, k2, startTime2, endTime2));
	
	std::printf("Example 3\n");
	// Example 3:
    int eventTime3 = 5, k3 = 2;
    std::vector<int> startTime3 = {0,1,2,3,4};
    std::vector<int> endTime3 = {1,2,3,4,5};
    std::printf("Example 3 Output: %d\n", maxFreeTime(eventTime3, k3, startTime3, endTime3));

    // Example 4:
	std::printf("Example 4\n");
    int eventTime4 = 64, k4 = 2;
    std::vector<int> startTime4 = {29, 49};
    std::vector<int> endTime4 = {37, 54};
    std::printf("Example 4 Output: %d\n", maxFreeTime(eventTime4, k4, startTime4, endTime4));

	
	// Example 5:
    int eventTime5 = 887, k5 = 24;
    std::vector<int> startTime5 = {61, 69, 71, 72, 99, 101, 103, 159, 374, 376, 406, 426, 449, 450, 453, 714, 770, 772, 778, 804, 811, 864, 866, 883, 884};
    std::vector<int> endTime5 = {68, 70, 72, 88, 101, 103, 135, 342, 376, 398, 417, 449, 450, 452, 705, 768, 772, 777, 803, 806, 863, 865, 883, 884, 886};
    std::printf("Output: %d\n", maxFreeTime(eventTime5, k5, startTime5, endTime5));
    return 0;
}
