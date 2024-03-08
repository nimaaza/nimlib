#pragma once

#include <string>
#include <vector>

#include "aggregations.h"

namespace nimlib::Metrics
{
    template <typename T>
    class Metric
    {
    public:
        using aggregator_ptr = std::shared_ptr<nimlib::Metrics::Aggregations::Aggregator<T>>;

        Metric(const std::string& name);
        ~Metric();

        Metric(const Metric&) = delete;
        Metric& operator=(const Metric&) = delete;
        Metric(Metric&&) noexcept = delete;
        Metric& operator=(Metric&&) noexcept = delete;

    public:
        Metric& register_aggregator(aggregator_ptr agg);
        bool receive(T m);
        const std::string& get_name() const;

    private:
        std::string name;
        std::vector<aggregator_ptr> aggregators{};
    };
};

namespace nimlib::Metrics
{
    template <typename T>
    Metric<T>::Metric(const std::string& name) : name{ name } {}

    template <typename T>
    Metric<T>::~Metric() {}

    template <typename T>
    Metric<T>& Metric<T>::register_aggregator(aggregator_ptr agg)
    {
        if (agg) aggregators.push_back(agg);
        return *this;
    }

    template <typename T>
    bool Metric<T>::receive(T m)
    {
        bool well_received{ true };

        for (auto agg : aggregators)
        {
            well_received = well_received && agg->involve(m);
        }

        return well_received;
    }

    template <typename T>
    const std::string& Metric<T>::get_name() const { return name; }
}
