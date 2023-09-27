#pragma once
#include <core/fwd.h>
#include <string>
#include <chrono>
#include <iostream>
#include <vector>
extern bool verbose;
class Timer
{
public:
    Timer(const std::string &name) : m_name(name)
    {
        m_startTimepoint = std::chrono::high_resolution_clock::now();
    }
    ~Timer()
    {
        if (!stopped)
            Stop();
    }
    void Stop()
    {
        auto endTimepoint = std::chrono::high_resolution_clock::now();
        auto start = std::chrono::time_point_cast<std::chrono::microseconds>(m_startTimepoint).time_since_epoch().count();
        auto end = std::chrono::time_point_cast<std::chrono::microseconds>(endTimepoint).time_since_epoch().count();
        auto duration = end - start;
        double ms = duration * 0.001;
        double s = ms * 0.001;
        if (verbose)
            printf("%s duration: %fs\n", m_name.c_str(), s);
        stopped = true;
    }

private:
    bool stopped = false;
    std::string m_name;
    std::chrono::time_point<std::chrono::high_resolution_clock> m_startTimepoint;
};
