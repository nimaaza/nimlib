#pragma once

#include "metrics_store.h"

#include <string>

namespace nimlib::Metrics::Measurements
{
    struct Measurement
    {
        Measurement(const std::string& target_name)
            : metric_store{ nimlib::Metrics::MetricsStore::get_instance().get_metric(target_name) }
        {};
        
        virtual ~Measurement()
        {
            if (metric_store)
            {
                metric_store->receive(measurement_result);
            }
        };

        // copy & move constructors?

    protected:
        nimlib::Metrics::metric_ptr metric_store;
        long measurement_result{};
    };

    class Count : public Measurement
    {
    public:
        Count(const std::string& target_name);
        ~Count();
    };

    class Duration : public Measurement
    {
    public:
        Duration(const std::string& target_name);
        ~Duration();

    private:
        Timer timer{};
    };
};
