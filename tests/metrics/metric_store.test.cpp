#include <gtest/gtest.h>

#include <vector>

#include "../../src/metrics/metric_store.h"
#include "../../src/metrics/metric.h"


using namespace nimlib::Metrics;

TEST(AggregatorTest, ReturnNullWhenMetricStoreNotRegistered)
{
    MetricsStore& agg{MetricsStore::get_instance() };
    std::string name{ "metric_1" };

    auto i = agg.get_metric(name);

    EXPECT_FALSE(i);
}

TEST(AggregatorTest, ReturnPointerToMetricStore)
{
    MetricsStore& agg{MetricsStore::get_instance() };
    std::string name_1{ "metric_1" };
    std::string name_2{ "metric_2" };
    auto ms_1 = std::make_shared<Metric>(name_1);
    auto ms_2 = std::make_shared<Metric>(name_2);

    agg.register_metric(ms_1);
    agg.register_metric(ms_2);
    auto r1 = agg.get_metric(name_1);
    auto r2 = agg.get_metric(name_2);

    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
}

TEST(AggregatorTest, AggregatorIsSingleton)
{
    MetricsStore& agg{MetricsStore::get_instance() };
    std::string name_1{ "metric_1" };
    std::string name_2{ "metric_2" };

    auto r1 = agg.get_metric(name_1);
    auto r2 = agg.get_metric(name_2);

    EXPECT_TRUE(r1);
    EXPECT_TRUE(r2);
}
