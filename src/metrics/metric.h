#pragma once

#include <string>
#include <vector>

#include "aggregations.h"

namespace nimlib::Server::Metrics
{
    template <typename T>
    class Metric
    {
    public:
        using aggregator_ptr = std::shared_ptr<Aggregations::Aggregator<T>>;

        explicit Metric(const std::string& name);
        ~Metric() = default;

        Metric(const Metric&) = delete;
        Metric& operator=(const Metric&) = delete;
        Metric(Metric&&) noexcept = delete;
        Metric& operator=(Metric&&) noexcept = delete;

    public:
        Metric& register_aggregator(aggregator_ptr agg);
        bool receive(T m);
        const std::string& get_name() const;
        std::string get_report() const;

    private:
        std::string name;
        std::vector<aggregator_ptr> aggregators{};
    };
};

namespace nimlib::Server::Metrics
{
    template <typename T>
    Metric<T>::Metric(const std::string& name) : name{ name } {}

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

        for (auto& agg : aggregators)
        {
            well_received = well_received && agg->involve(m);
        }

        return well_received;
    }

    template <typename T>
    const std::string& Metric<T>::get_name() const { return name; }

    template <typename T>
    std::string Metric<T>::get_report() const
    {
        return "";
    }
}
