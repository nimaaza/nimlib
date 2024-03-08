#pragma once

#include "metrics_store.h"

#include <string>
#include <memory>

namespace nimlib::Metrics::Measurements
{
    template <typename T>
    struct Measurement
    {
        Measurement(const std::string& target_name)
            : metric{ nimlib::Metrics::MetricsStore<T>::get_instance().get_metric(target_name) }
        {};

        virtual ~Measurement()
        {
            if (metric)
            {
                metric->receive(measurement_result);
            }
        };

        // copy & move constructors?

    protected:
        std::shared_ptr<nimlib::Metrics::Metric<T>> metric;
        T measurement_result{};
    };

    template <typename T>
    class Count : public Measurement<T>
    {
    public:
        Count(const std::string& target_name);
        ~Count();
    };

    template <typename T>
    class Duration : public Measurement<T>
    {
    public:
        Duration(const std::string& target_name);
        ~Duration();

    private:
        Timer timer{};
    };
};

namespace nimlib::Metrics::Measurements
{
    template <typename T>
    Count<T>::Count(const std::string& target_name) : Measurement<T>{ target_name } {};

    template <typename T>
    Count<T>::~Count() { Measurement<T>::measurement_result = 1; }

    template <typename T>
    Duration<T>::Duration(const std::string& target_name) : Measurement<T>{ target_name } { timer.begin(); };

    template <typename T>
    Duration<T>::~Duration()
    {
        long duration;
        if (timer.end(duration)) Measurement<T>::measurement_result = duration;

    }
}
