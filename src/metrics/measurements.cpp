#include "measurements.h"

namespace nimlib::Metrics::Measurements
{
    Count::Count(const std::string& target_name) : Measurement{ target_name } {};

    Count::~Count() { measurement_result = 1; }

    Duration::Duration(const std::string& target_name) : Measurement{ target_name } { timer.begin(); };

    Duration::~Duration()
    {
        long duration;
        if (timer.end(duration)) measurement_result = duration;

    }
}
