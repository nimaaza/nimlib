#include "metric_store.h"

namespace nimlib::Metrics
{
    MetricsStore::MetricsStore() {}

    MetricsStore::~MetricsStore() {}


    void MetricsStore::register_metric(metric_ptr ms)
    {
        metric_register.insert_or_assign(ms->get_name(), ms);
    }

    metric_ptr MetricsStore::get_metric(const std::string& name)
    {
        auto i = metric_register.find(name);
        return (i != metric_register.end()) ? i->second : nullptr;
    }

    void MetricsStore::generate_stats_report() {} // TODO

    MetricsStore& MetricsStore::get_instance()
    {
        static MetricsStore aggregator_instance{};
        return aggregator_instance;
    }
}
