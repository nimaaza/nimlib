#pragma once

#include "metric_store.h"

#include <string>

namespace nimlib::Metrics::Measurements
{
    struct Measurement
    {
        Measurement(const std::string& target_name)
            : metric_store{nimlib::Metrics::MetricsStore::get_instance().get_metric(target_name) }
        {};
        virtual ~Measurement() = default;

        // copy & move constructors?

    protected:
        nimlib::Metrics::metric_ptr metric_store;
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
