#pragma once

#include "../common/types.h"
#include "metric.h"
#include "metrics_store.h"

#include <string>
#include <memory>

namespace nimlib::Server::Metrics
{
    template <typename T = long>
    class Builder
    {
        using metric_ptr = std::shared_ptr<Metric<T>>;
    private:
        explicit Builder(const std::string& metric_name);

    public:
        ~Builder() = default;

        Builder(const Builder&) = delete;
        Builder& operator=(const Builder&) = delete;
        Builder(Builder&&) noexcept = default;
        Builder& operator=(Builder&&) noexcept = delete;

        Builder& measure_increment();
        Builder& measure_avg();
        Builder& measure_max();
        Builder& measure_min();
        Builder& measure_sum();
        Builder& measure_med();
        Builder& measure_avg_rate();
        Builder& with_timeseries(int capacity);

        metric_ptr build();

        static Builder instantiate_metric(const std::string& metric_name);

    private:
        metric_ptr metric;
    };
};

namespace nimlib::Server::Metrics
{
    template <typename T>
    Builder<T>::Builder(const std::string& metric_name)
    {
        metric = std::make_shared<PointMetric<T>>(metric_name);
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_increment()
    {
        auto inc = std::make_shared<Aggregations::Increment<T>>();
        metric->register_aggregator(inc);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_avg()
    {
        auto avg = std::make_shared<Aggregations::Avg<T>>();
        metric->register_aggregator(avg);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_max()
    {
        auto max = std::make_shared<Aggregations::Max<T>>();
        metric->register_aggregator(max);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_min()
    {
        auto min = std::make_shared<Aggregations::Min<T>>();
        metric->register_aggregator(min);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_sum()
    {
        auto sum = std::make_shared<Aggregations::Sum<T>>();
        metric->register_aggregator(sum);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_med()
    {
        auto med = std::make_shared<Aggregations::Med<T>>();
        metric->register_aggregator(med);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::measure_avg_rate()
    {
        auto avg_rate = std::make_shared<Aggregations::AvgRatePerSecond<T>>();
        metric->register_aggregator(avg_rate);
        return *this;
    }

    template <typename T>
    Builder<T>& Builder<T>::with_timeseries(int capacity)
    {
        metric = std::make_shared<TimeSeriesMetric<T>>(metric, capacity);
        return *this;
    }

    template <typename T>
    std::shared_ptr<Metric<T>> Builder<T>::build()
    {
        auto& metrics_store = MetricsStore<T>::get_instance();
        metrics_store.register_metric(metric);
        return metric;
    }

    template <typename T>
    Builder<T> Builder<T>::instantiate_metric(const std::string& metric_name)
    {
        Builder metric_factory{ metric_name };
        return metric_factory;
    }
};
