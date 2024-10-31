#include "ImGui/imgui.h"
#include <chrono>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma once

template<typename Fn>
class Timer
{
public:
    Timer(const char* name, Fn&& func)
        :m_name(name), m_func(func), m_stopped(false)
    {
        m_startTimePoint = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        if (!m_stopped)
            stop();
    }

    void stop()
    {
        auto endTimePoint = std::chrono::high_resolution_clock::now();

        long long start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimePoint).time_since_epoch().count();
        long long end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimePoint).time_since_epoch().count();

        m_stopped = true;

        float duration = (end - start) * 0.001f;
        m_func({ m_name, duration });
    }

private:
    const char* m_name;
    Fn m_func;
    std::chrono::time_point<std::chrono::steady_clock> m_startTimePoint;
    bool m_stopped;
};

