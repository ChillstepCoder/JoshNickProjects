#pragma once
#include <chrono>
#include "Profiler.h"

class Timer
{
public:
    Timer(const char* name)
        : m_Name(name), m_Stopped(false)
    {
        m_StartTimepoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        if (!m_Stopped)
            Stop();
    }

    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_StartTimepoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();

        float duration = (end - start) * 0.001f; // Convert to milliseconds

        ProfileResult result;
        result.Name = m_Name;
        result.Time = duration;

        // Submit to global profiler
        Profiler::Get().AddResult(result);

        m_Stopped = true;
    }

private:
    const char* m_Name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_StartTimepoint;
    bool m_Stopped;
};