/*
 Reschedule Meetings for Maximum Free Time I:
 The key idea is to:
  Identify gaps between meetings and the event boundaries.
  Try rescheduling up to k meetings to maximize the largest gap.
  Use a sliding window approach to consider valid rescheduling positions while
  ensuring meetings remain within [0, eventTime] and maintain their relative
 order. Explanation: Data Structure: We create a vector of pairs to store
 meeting start and end times, ensuring we can sort them if needed (though the
 problem states they are non-overlapping and ordered). Initial Gaps: Calculate
 gaps between consecutive meetings, including the gap from time 0 to the first
 meeting and from the last meeting to eventTime. Rescheduling Strategy: For each
 meeting, we try moving it to its earliest possible start time (right after the
 previous meeting or 0 if it's the first) and its latest possible start time
 (right before the next meeting or eventTime if it's the last). For each move,
 we recalculate the affected gaps (before and after the moved meeting) and find
 the maximum gap possible. We only consider up to k moves, and for simplicity,
 this solution evaluates single moves (since k=1 in the examples).
*/

#include <algorithm>
#include <cstdio>
#include <functional>
#include <vector>

int maxFreeTime_recursive(int eventTime, int k, std::vector<int> &startTime,
                          std::vector<int> &endTime) {
  int n = startTime.size();
  std::vector<std::pair<int, int>> meetings(n);
  for (int i = 0; i < n; ++i) {
    meetings[i] = {startTime[i], endTime[i]};
  }

  // Sort meetings by start time to ensure order
  std::sort(meetings.begin(), meetings.end());

  // Calculate initial gaps
  std::vector<int> gaps;
  int prevEnd = 0;
  for (int i = 0; i < n; ++i) {
    gaps.push_back(meetings[i].first - prevEnd);
    prevEnd = meetings[i].second;
  }
  gaps.push_back(eventTime - prevEnd);

  int maxFree = *std::max_element(gaps.begin(), gaps.end());

  // Try all combinations of up to k moves
  std::vector<int> currentStarts = startTime;
  std::function<void(int, int, std::vector<int> &)> tryCombinations =
      [&](int pos, int movesLeft, std::vector<int> &starts) {
        if (movesLeft == 0 || pos >= n) {
          // Calculate gaps for current configuration
          std::vector<int> tempGaps;
          int prevEnd = 0;
          for (int i = 0; i < n; ++i) {
            tempGaps.push_back(starts[i] - prevEnd);
            prevEnd = meetings[i].second - (meetings[i].first - starts[i]);
          }
          tempGaps.push_back(eventTime - prevEnd);
          maxFree = std::max(
              maxFree, *std::max_element(tempGaps.begin(), tempGaps.end()));
          return;
        }

        // Don't move meeting pos
        tryCombinations(pos + 1, movesLeft, starts);

        // Try moving meeting pos to earliest possible position
        int earliestStart =
            (pos == 0) ? 0
                       : meetings[pos - 1].second -
                             (meetings[pos - 1].first - starts[pos - 1]);
        int duration = meetings[pos].second - meetings[pos].first;
        if (starts[pos] > earliestStart) {
          int originalStart = starts[pos];
          starts[pos] = earliestStart;
          tryCombinations(pos + 1, movesLeft - 1, starts);
          starts[pos] = originalStart; // Backtrack
        }

        // Try moving meeting pos to latest possible position
        int nextStart = (pos == n - 1) ? eventTime - duration : starts[pos + 1];
        int latestStart = nextStart - duration;
        if (starts[pos] < latestStart) {
          int originalStart = starts[pos];
          starts[pos] = latestStart;
          tryCombinations(pos + 1, movesLeft - 1, starts);
          starts[pos] = originalStart; // Backtrack
        }
      };

  tryCombinations(0, k, currentStarts);

  return maxFree;
}

int maxFreeTime(int eventTime, int k, std::vector<int> &startTime,
                std::vector<int> &endTime) {
  int n = startTime.size();
  std::vector<int> starts = startTime; // Single copy of start times
  int maxFree = 0;

  // Calculate initial maximum gap
  int prevEnd = 0;
  for (int i = 0; i < n; ++i) {
    maxFree = std::max(maxFree, startTime[i] - prevEnd);
    prevEnd = endTime[i];
  }
  maxFree = std::max(maxFree, eventTime - prevEnd);

  // Try sliding meetings left (earliest possible positions)
  for (int start = 0; start < n; ++start) {
    int moves = k;
    std::vector<int> tempStarts =
        startTime; // Temporary copy for this iteration
    int prevEnd = 0;
    bool valid = true;

    // Move meetings from 'start' onward to earliest positions
    for (int i = start; i < n && moves > 0; ++i) {
      int duration = endTime[i] - startTime[i];
      int earliestStart =
          (i == 0) ? 0
                   : endTime[i - 1] - (startTime[i - 1] - tempStarts[i - 1]);
      if (tempStarts[i] > earliestStart) {
        tempStarts[i] = earliestStart;
        --moves;
      }
      if (i > 0 && tempStarts[i] < endTime[i - 1] -
                                       (startTime[i - 1] - tempStarts[i - 1])) {
        valid = false; // Overlap detected
        break;
      }
    }

    if (valid) {
      // Calculate gaps
      int currentMax = 0;
      prevEnd = 0;
      for (int i = 0; i < n; ++i) {
        currentMax = std::max(currentMax, tempStarts[i] - prevEnd);
        prevEnd = endTime[i] - (startTime[i] - tempStarts[i]);
      }
      currentMax = std::max(currentMax, eventTime - prevEnd);
      maxFree = std::max(maxFree, currentMax);
    }
  }

  // Try sliding meetings right (latest possible positions)
  for (int end = n - 1; end >= 0; --end) {
    int moves = k;
    std::vector<int> tempStarts =
        startTime; // Temporary copy for this iteration
    int nextStart = eventTime;
    bool valid = true;

    // Move meetings from 'end' backward to latest positions
    for (int i = end; i >= 0 && moves > 0; --i) {
      int duration = endTime[i] - startTime[i];
      int latestStart =
          (i == n - 1) ? eventTime - duration : tempStarts[i + 1] - duration;
      if (tempStarts[i] < latestStart) {
        tempStarts[i] = latestStart;
        --moves;
      }
      if (i < n - 1 && tempStarts[i] + duration > tempStarts[i + 1]) {
        valid = false; // Overlap detected
        break;
      }
    }

    if (valid) {
      // Calculate gaps
      int currentMax = 0;
      prevEnd = 0;
      for (int i = 0; i < n; ++i) {
        currentMax = std::max(currentMax, tempStarts[i] - prevEnd);
        prevEnd = endTime[i] - (startTime[i] - tempStarts[i]);
      }
      currentMax = std::max(currentMax, eventTime - prevEnd);
      maxFree = std::max(maxFree, currentMax);
    }
  }

  return maxFree;
}

int main() {
  std::printf("Compile: g++ -std=c++11 -o max_free_time max_free_time.cpp \n");
  std::printf("Example 1\n");
  // Example 1:
  int eventTime1 = 5, k1 = 1;
  std::vector<int> startTime1 = {1, 3};
  std::vector<int> endTime1 = {2, 5};
  std::printf("Example 1 Output: %d\n",
              maxFreeTime(eventTime1, k1, startTime1, endTime1));

  std::printf("Example 2\n");
  // Example 2:
  int eventTime2 = 10, k2 = 1;
  std::vector<int> startTime2 = {0, 2, 9};
  std::vector<int> endTime2 = {1, 4, 10};
  std::printf("Example 2 Output: %d\n",
              maxFreeTime(eventTime2, k2, startTime2, endTime2));

  std::printf("Example 3\n");
  // Example 3:
  int eventTime3 = 5, k3 = 2;
  std::vector<int> startTime3 = {0, 1, 2, 3, 4};
  std::vector<int> endTime3 = {1, 2, 3, 4, 5};
  std::printf("Example 3 Output: %d\n",
              maxFreeTime(eventTime3, k3, startTime3, endTime3));

  // Example 4:
  std::printf("Example 4\n");
  int eventTime4 = 64, k4 = 2;
  std::vector<int> startTime4 = {29, 49};
  std::vector<int> endTime4 = {37, 54};
  std::printf("Example 4 Output: %d\n",
              maxFreeTime(eventTime4, k4, startTime4, endTime4));

  // Example 5:
  int eventTime5 = 887, k5 = 24;
  std::vector<int> startTime5 = {61,  69,  71,  72,  99,  101, 103, 159, 374,
                                 376, 406, 426, 449, 450, 453, 714, 770, 772,
                                 778, 804, 811, 864, 866, 883, 884};
  std::vector<int> endTime5 = {68,  70,  72,  88,  101, 103, 135, 342, 376,
                               398, 417, 449, 450, 452, 705, 768, 772, 777,
                               803, 806, 863, 865, 883, 884, 886};
  std::printf("Output: %d\n",
              maxFreeTime(eventTime5, k5, startTime5, endTime5));
  return 0;
}
