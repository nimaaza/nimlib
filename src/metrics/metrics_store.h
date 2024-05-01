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
        MetricsStore() = default;

    public:
        ~MetricsStore() = default;

        MetricsStore(const MetricsStore&) = delete;
        MetricsStore& operator=(const MetricsStore&) = delete;
        MetricsStore(MetricsStore&&) noexcept = delete;
        MetricsStore& operator=(MetricsStore&&) noexcept = delete;

    public:
        void register_metric(metric_ptr m);
        metric_ptr get_metric(const std::string& name);
        std::string generate_stats_report();

        static MetricsStore& get_instance();

    private:
        std::unordered_map<std::string, metric_ptr> metrics_register{};
    };
};

namespace nimlib::Server::Metrics
{
    template <typename T>
    using metric_ptr = std::shared_ptr<nimlib::Server::Metrics::Metric<T>>;

    template <typename T>
    void MetricsStore<T>::register_metric(metric_ptr m)
    {
        metrics_register.insert_or_assign(m->get_name(), m);
    }

    template <typename T>
    metric_ptr<T> MetricsStore<T>::get_metric(const std::string& name)
    {
        auto it = metrics_register.find(name);
        return (it != metrics_register.end()) ? it->second : nullptr;
    }

    template <typename T>
    std::string MetricsStore<T>::generate_stats_report()
    {
        std::stringstream report{};
        for (const auto& [name, metric] : metrics_register)
        {
            report << metric->get_report();
        }
    }

    template <typename T>
    MetricsStore<T>& MetricsStore<T>::get_instance()
    {
        static MetricsStore aggregator_instance{};
        return aggregator_instance;
    }
}
