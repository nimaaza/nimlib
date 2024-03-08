#include <gtest/gtest.h>

#include <vector>

#include "../../src/metrics/metrics_store.h"

using namespace nimlib::Metrics;

TEST(MetricStoreTest, ReturnNullWhenMetricStoreNotRegistered)
{
    MetricsStore& metric_store{MetricsStore::get_instance() };
    std::string name{ "metric_1" };

    auto i = metric_store.get_metric(name);

    EXPECT_FALSE(i);
}

TEST(MetricStoreTest, ReturnPointerToMetricStore)
{
    MetricsStore& metric_store{MetricsStore::get_instance() };
    std::string name_1{ "metric_1" };
    std::string name_2{ "metric_2" };
    auto ms_1 = std::make_shared<Metric>(name_1);
    auto ms_2 = std::make_shared<Metric>(name_2);

    metric_store.register_metric(ms_1);
    metric_store.register_metric(ms_2);
    auto r1 = metric_store.get_metric(name_1);
    auto r2 = metric_store.get_metric(name_2);

    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
}

TEST(MetricStoreTest, AggregatorIsSingleton)
{
    MetricsStore& metric_store{MetricsStore::get_instance() };
    std::string name_1{ "metric_1" };
    std::string name_2{ "metric_2" };

    auto r1 = metric_store.get_metric(name_1);
    auto r2 = metric_store.get_metric(name_2);

    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
}
