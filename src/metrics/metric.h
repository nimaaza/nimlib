#pragma once

#include <string>
#include <vector>

#include "aggregations.h"

namespace nimlib::Metrics
{
    using aggregator_ptr = std::shared_ptr<nimlib::Metrics::Aggregations::Opt>;

    class Metric
    {
    public:
        Metric(const std::string& name);
        ~Metric();

        Metric(const Metric&) = delete;
        Metric& operator=(const Metric&) = delete;
        Metric(Metric&&) noexcept = delete;
        Metric& operator=(Metric&&) noexcept = delete;

    public:
        Metric& register_aggregator(aggregator_ptr agg);
        bool receive(size_t m);
        const std::string& get_name() const;

    private:
        std::string name;
        std::vector<aggregator_ptr> aggregators{};
    };
};
