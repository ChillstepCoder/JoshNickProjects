// PerformanceTimer.h
#pragma once
#include <string>
#include <chrono>
#include <unordered_map>
#include <vector>
#include <cstdint>
#include <memory>
#include <cfloat>

// Forward declare the timer class
class PerformanceTimer;

// Simple RAII-style timer class
class ScopedTimer {
public:
  explicit ScopedTimer(const std::string& name, PerformanceTimer& timer);
  ~ScopedTimer();

private:
  std::string m_name;
  PerformanceTimer& m_timer;
};

class PerformanceTimer {
public:
  struct TimingData {
    float lastTime = 0.0f;         // Last frame's time
    float averageTime = 0.0f;      // Moving average
    float minTime = FLT_MAX;       // Minimum recorded time
    float maxTime = 0.0f;          // Maximum recorded time
    uint32_t frameCount = 0;       // Number of frames measured
    static const int HISTORY_SIZE = 60;  // Keep last 60 frames for average
    std::vector<float> history;    // Circular buffer of recent timings

    TimingData() : history(HISTORY_SIZE, 0.0f) {}
  };

  static PerformanceTimer& getInstance() {
    static PerformanceTimer instance;
    return instance;
  }

  void startTimer(const std::string& name) {
    auto& timer = m_timers[name];
    timer.startTime = std::chrono::high_resolution_clock::now();
  }

  void endTimer(const std::string& name) {
    auto endTime = std::chrono::high_resolution_clock::now();
    auto& timer = m_timers[name];

    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(
      endTime - timer.startTime).count() / 1000.0f;  // Convert to milliseconds

    // Update timing data
    auto& data = m_timingData[name];
    data.lastTime = duration;
    data.minTime = std::min<float>(data.minTime, duration);
    data.maxTime = std::max<float>(data.maxTime, duration);

    // Update history buffer
    data.history[data.frameCount % TimingData::HISTORY_SIZE] = duration;
    data.frameCount++;

    // Calculate moving average
    float sum = 0.0f;
    int count = std::min<float>(TimingData::HISTORY_SIZE, static_cast<int>(data.frameCount));
    for (int i = 0; i < count; i++) {
      sum += data.history[i];
    }
    data.averageTime = sum / count;
  }

  const TimingData& getTimingData(const std::string& name) const {
    static TimingData emptyData;
    auto it = m_timingData.find(name);
    return (it != m_timingData.end()) ? it->second : emptyData;
  }

  void reset() {
    m_timers.clear();
    m_timingData.clear();
  }

private:
  friend class ScopedTimer;

  PerformanceTimer() = default;

  struct Timer {
    std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
  };

  std::unordered_map<std::string, Timer> m_timers;
  std::unordered_map<std::string, TimingData> m_timingData;
};

inline ScopedTimer::ScopedTimer(const std::string& name, PerformanceTimer& timer)
  : m_name(name), m_timer(timer) {
  m_timer.startTimer(m_name);
}

inline ScopedTimer::~ScopedTimer() {
  m_timer.endTimer(m_name);
}

// Macro that creates a scoped timer
#define TIME_SCOPE(name) ScopedTimer scopedTimer##__LINE__(name, PerformanceTimer::getInstance())
