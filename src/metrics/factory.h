#pragma once

#include "metric.h"

#include <string>
#include <memory>

namespace nimlib::Metrics
{
    using metric_ptr = std::shared_ptr<nimlib::Metrics::Metric>;

    class Factory
    {
    private:
        Factory(const std::string& metric_name);

    public:
        ~Factory();

        Factory& measure_increment();

        Factory& measure_avg();

        Factory& measure_max();

        Factory& measure_min();

        Factory& measure_sum();

        metric_ptr get();

        static Factory instanciate_metric(const std::string& metric_name);

    private:
        metric_ptr metric;
    };
};
