#pragma once

#include <string>
#include <unordered_map>
#include <memory>

#include "metric.h"

namespace nimlib::Metrics
{
    using metric_ptr = std::shared_ptr<nimlib::Metrics::Metric>;

    class MetricsStore
    {
    private:
        MetricsStore();

    public:
        ~MetricsStore();

        MetricsStore(const MetricsStore&) = delete;
        MetricsStore& operator=(const MetricsStore&) = delete;
        MetricsStore(MetricsStore&&) noexcept = delete;
        MetricsStore& operator=(MetricsStore&&) noexcept = delete;

    public:
        void register_metric(metric_ptr ms);
        metric_ptr get_metric(const std::string& name);
        void generate_stats_report(); // TODO
        
        static MetricsStore& get_instance();

    private:
        std::unordered_map<std::string, metric_ptr> metric_register{};
    };
};
