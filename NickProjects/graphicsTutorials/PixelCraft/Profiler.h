#pragma once

#define PROFILE_SCOPE(name) Timer timer##__LINE__(name,[&](ProfileResult profileResult) { m_profileResults.push_back(profileResult); })

struct ProfileResult
{
    const char* Name;
    float Time;
};
static std::vector<ProfileResult> m_profileResults;