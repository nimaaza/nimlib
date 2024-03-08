#include "metric.h"

namespace nimlib::Metrics
{
    Metric::Metric(const std::string& name) : name{ name } {}

    Metric::~Metric() {}

    Metric& Metric::register_aggregator(aggregator_ptr agg)
    {
        if (agg) aggregators.push_back(agg);
        return *this;
    }

    bool Metric::receive(size_t m)
    {
        bool well_received{ true };

        for (auto agg : aggregators)
        {
            well_received = well_received && agg->involve(m);
        }

        return well_received;
    }

    const std::string& Metric::get_name() const { return name; }
}
