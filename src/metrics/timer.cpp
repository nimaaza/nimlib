#include "timer.h"

nimlib::Metrics::Measurements::Timer::Timer() : then{ std::chrono::high_resolution_clock::now() } {}

nimlib::Metrics::Measurements::Timer::~Timer() {}

bool nimlib::Metrics::Measurements::Timer::begin()
{
    if (timing)
    {
        return false;
    }
    else
    {
        timing = true;
        then = std::chrono::high_resolution_clock::now();
        return true;
    }
}

bool nimlib::Metrics::Measurements::Timer::end(long& latency)
{
    if (!timing)
    {
        return false;
    }
    else
    {
        auto now = std::chrono::high_resolution_clock::now();
        latency = std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count();
        timing = false;
        return true;
    }
}
