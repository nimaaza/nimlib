#include "timer.h"

namespace nimlib::Server::Utils
{
    Timer::Timer() : then{ std::chrono::steady_clock::now() } {}

    bool Timer::begin()
    {
        if (timing)
        {
            return false;
        }
        else
        {
            timing = true;
            then = std::chrono::steady_clock::now();
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
            auto now = std::chrono::steady_clock::now();
            duration = std::chrono::duration_cast<std::chrono::nanoseconds>(now - then).count();
            timing = false;
            return true;
        }
    }
}
