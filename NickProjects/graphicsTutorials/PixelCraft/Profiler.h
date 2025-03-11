#pragma once
#define PROFILE_SCOPE(name) Timer timer##__LINE__(name)
#include <vector>
#include <string>
#include <unordered_map>

struct ProfileResult
{
    const char* Name;
    float Time;
    float maxTime = 0.0f; // Optional: can move this to the Profiler class instead
};

class Profiler
{
private:
    static Profiler* s_Instance;
    std::vector<ProfileResult> m_ProfileResults;
    std::unordered_map<std::string, float> m_MaxTimes;

    // Private constructor for singleton
    Profiler() {}

public:
    static Profiler& Get()
    {
        if (!s_Instance)
            s_Instance = new Profiler();
        return *s_Instance;
    }

    std::unordered_map<std::string, ProfileResult> m_LatestResults;

    void AddResult(const ProfileResult& result)
    {
        // Always update the latest result for this name
        m_LatestResults[result.Name] = result;

        // Update max time
        if (result.Time > m_MaxTimes[result.Name])
            m_MaxTimes[result.Name] = result.Time;
    }

    const std::unordered_map<std::string, ProfileResult>& GetLatestResults() const
    {
        return m_LatestResults;
    }


    const std::vector<ProfileResult>& GetResults() const { return m_ProfileResults; }
    const std::unordered_map<std::string, float>& GetMaxTimes() const { return m_MaxTimes; }

    void ClearResults() { m_ProfileResults.clear(); }
};