#include <chrono>

namespace nimlib::Server::Metrics::Measurements
{
    class Timer
    {
    public:
        Timer();
        ~Timer() = default;

        Timer(const Timer&) = delete;
        Timer& operator=(const Timer&) = delete;
        Timer(Timer&&) noexcept = delete;
        Timer& operator=(Timer&&) noexcept = delete;

        bool begin();
        bool end(long& duration);

    private:
        std::chrono::high_resolution_clock::time_point then;
        bool timing{ false };
    };
}
