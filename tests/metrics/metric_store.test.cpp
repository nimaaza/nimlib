#include <gtest/gtest.h>

#include <vector>

#include "../../src/metrics/metrics_store.h"

using namespace nimlib::Metrics;

TEST(MetricStoreTest, ReturnNullWhenMetricStoreNotRegistered)
{
    MetricsStore<long>& metric_store{MetricsStore<long>::get_instance() };
    std::string name{ "metric_1" };

    auto i = metric_store.get_metric(name);

    EXPECT_FALSE(i);
}

TEST(MetricStoreTest, ReturnPointerToMetricStore)
{
    MetricsStore<long>& metric_store{MetricsStore<long>::get_instance() };
    std::string name_1{ "metric_1" };
    std::string name_2{ "metric_2" };
    auto ms_1 = std::make_shared<Metric<long>>(name_1);
    auto ms_2 = std::make_shared<Metric<long>>(name_2);

    metric_store.register_metric(ms_1);
    metric_store.register_metric(ms_2);
    auto r1 = metric_store.get_metric(name_1);
    auto r2 = metric_store.get_metric(name_2);

    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
}

TEST(MetricStoreTest, AggregatorIsSingleton)
{
    MetricsStore<long>& metric_store{MetricsStore<long>::get_instance() };
    std::string name_1{ "metric_1" };
    std::string name_2{ "metric_2" };

    auto r1 = metric_store.get_metric(name_1);
    auto r2 = metric_store.get_metric(name_2);

    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
}
