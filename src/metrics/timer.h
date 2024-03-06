#include <chrono>

namespace nimlib::Metrics::Measurements
{
    class Timer
    {
    public:
        Timer();
        ~Timer();

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;
        Timer(Timer&&) noexcept = delete;
        Timer& operator=(Timer&&) noexcept = delete;

        bool begin();
        bool end(long& latency);

    private:
        std::chrono::high_resolution_clock::time_point then;
        bool timing{ false };
    };
}
