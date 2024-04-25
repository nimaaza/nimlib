#include "timer.h"

namespace nimlib::Server::Metrics::Measurements
{
    Timer::Timer() : then{ std::chrono::high_resolution_clock::now() } {}

    bool Timer::begin()
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

    bool Timer::end(long& duration)
    {
        if (!timing)
        {
            return false;
        }
        else
        {
            auto now = std::chrono::high_resolution_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count();
            timing = false;
            return true;
        }
    }
}
