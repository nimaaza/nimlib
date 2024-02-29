#include "timer.h"

nimlib::Logger::Timer::Timer() : then{ std::chrono::high_resolution_clock::now() } {}

nimlib::Logger::Timer::~Timer() {}

bool nimlib::Logger::Timer::begin()
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

bool nimlib::Logger::Timer::end(long& latency)
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
