#include "factory.h"

#include "metrics_store.h"

namespace nimlib::Metrics
{
    Factory::Factory(const std::string& metric_name)
    {
        metric = std::make_shared<Metric>(metric_name);
    }

    Factory::~Factory() {}

    Factory& Factory::measure_increment()
    {
        auto inc = std::make_shared<nimlib::Metrics::Aggregations::Increment>();
        metric->register_aggregator(inc);
        return *this;
    }

    Factory& Factory::measure_avg()
    {
        auto avg = std::make_shared<nimlib::Metrics::Aggregations::Avg>();
        metric->register_aggregator(avg);
        return *this;
    }

    Factory& Factory::measure_max()
    {
        auto max = std::make_shared<nimlib::Metrics::Aggregations::Max>();
        metric->register_aggregator(max);
        return *this;
    }


    Factory& Factory::measure_min()
    {
        auto min = std::make_shared<nimlib::Metrics::Aggregations::Min>();
        metric->register_aggregator(min);
        return *this;
    }


    Factory& Factory::measure_sum()
    {
        auto sum = std::make_shared<nimlib::Metrics::Aggregations::Sum>();
        metric->register_aggregator(sum);
        return *this;
    }

    metric_ptr Factory::get()
    {
        auto& metrics_store = nimlib::Metrics::MetricsStore::get_instance();
        metrics_store.register_metric(metric);
        return metric;
    }

    Factory Factory::instanciate_metric(const std::string& metric_name)
    {
        Factory metric_factory{ metric_name };
        return metric_factory;
    }
};
