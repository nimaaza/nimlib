#include "measurements.h"

#include<iostream>

namespace nimlib::Metrics::Measurements
{
    Count::Count(const std::string& target_name) : Measurement{ target_name } {};

    Count::~Count() { metric_store->receive(1); }

    Duration::Duration(const std::string& target_name) : Measurement{ target_name }
    {
        timer.begin();
    };

    Duration::~Duration()
    {
        long duration;
        if (timer.end(duration))
        {
            std::cout << duration << std::endl;
            metric_store->receive(duration);
        }
    }
}
