#pragma once

#include "metric.h"
#include "metrics_store.h"

#include <string>
#include <memory>

namespace nimlib::Metrics
{
    template <typename T = long>
    class Factory
    {
        using metric_ptr = std::shared_ptr<nimlib::Metrics::Metric<T>>;
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

namespace nimlib::Metrics
{
    template <typename T>
    Factory<T>::Factory(const std::string& metric_name)
    {
        metric = std::make_shared<Metric<T>>(metric_name);
    }

    template <typename T>
    Factory<T>::~Factory() {}

    template <typename T>
    Factory<T>& Factory<T>::measure_increment()
    {
        auto inc = std::make_shared<nimlib::Metrics::Aggregations::Increment<T>>();
        metric->register_aggregator(inc);
        return *this;
    }

    template <typename T>
    Factory<T>& Factory<T>::measure_avg()
    {
        auto avg = std::make_shared<nimlib::Metrics::Aggregations::Avg<T>>();
        metric->register_aggregator(avg);
        return *this;
    }

    template <typename T>
    Factory<T>& Factory<T>::measure_max()
    {
        auto max = std::make_shared<nimlib::Metrics::Aggregations::Max<T>>();
        metric->register_aggregator(max);
        return *this;
    }

    template <typename T>
    Factory<T>& Factory<T>::measure_min()
    {
        auto min = std::make_shared<nimlib::Metrics::Aggregations::Min<T>>();
        metric->register_aggregator(min);
        return *this;
    }

    template <typename T>
    Factory<T>& Factory<T>::measure_sum()
    {
        auto sum = std::make_shared<nimlib::Metrics::Aggregations::Sum<T>>();
        metric->register_aggregator(sum);
        return *this;
    }

    template <typename T>
    std::shared_ptr<nimlib::Metrics::Metric<T>> Factory<T>::get()
    {
        auto& metrics_store = nimlib::Metrics::MetricsStore<T>::get_instance();
        metrics_store.register_metric(metric);
        return metric;
    }

    template <typename T>
    Factory<T> Factory<T>::instanciate_metric(const std::string& metric_name)
    {
        Factory metric_factory{ metric_name };
        return metric_factory;
    }
};
