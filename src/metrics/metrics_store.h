#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "metric.h"

namespace nimlib::Server::Metrics
{
    template <typename T>
    class MetricsStore
    {
        using metric_ptr = std::shared_ptr<nimlib::Server::Metrics::Metric<T>>;
    private:
        MetricsStore();

    public:
        ~MetricsStore();

        MetricsStore(const MetricsStore&) = delete;
        MetricsStore& operator=(const MetricsStore&) = delete;
        MetricsStore(MetricsStore&&) noexcept = delete;
        MetricsStore& operator=(MetricsStore&&) noexcept = delete;

    public:
        void register_metric(metric_ptr m);
        metric_ptr get_metric(const std::string& name);
        void generate_stats_report(); // TODO

        static MetricsStore& get_instance();

    private:
        std::unordered_map<std::string, metric_ptr> metric_register{};
    };
};

namespace nimlib::Server::Metrics
{
    template <typename T>
    using metric_ptr = std::shared_ptr<nimlib::Server::Metrics::Metric<T>>;

    template <typename T>
    MetricsStore<T>::MetricsStore() {}

    template <typename T>
    MetricsStore<T>::~MetricsStore() {}

    template <typename T>
    void MetricsStore<T>::register_metric(metric_ptr m)
    {
        metric_register.insert_or_assign(m->get_name(), m);
    }

    template <typename T>
    metric_ptr<T> MetricsStore<T>::get_metric(const std::string& name)
    {
        auto i = metric_register.find(name);
        return (i != metric_register.end()) ? i->second : nullptr;
    }

    template <typename T>
    void MetricsStore<T>::generate_stats_report() {} // TODO

    template <typename T>
    MetricsStore<T>& MetricsStore<T>::get_instance()
    {
        static MetricsStore aggregator_instance{};
        return aggregator_instance;
    }
}
