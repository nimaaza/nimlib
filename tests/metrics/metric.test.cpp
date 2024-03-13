#include <gtest/gtest.h>

#include <vector>

#include "../../src/metrics/metrics_store.h"
#include "../../src/metrics/aggregations.h"
#include "../../src/metrics/metric.h"

using namespace nimlib::Server::Metrics;

TEST(MetricStoreTest, _)
{
    std::vector<int> values{ 4, 6, 8, 12, 45, 67, 8, 0, 9, -1, -2, 4, 7, -10 };
    auto avg = std::make_shared<Aggregations::Avg<long>>();
    auto max = std::make_shared<Aggregations::Max<long>>();
    auto min = std::make_shared<Aggregations::Min<long>>();
    auto sum = std::make_shared<Aggregations::Sum<long>>();
    long avg_result{};
    long max_result{};
    long min_result{};
    long sum_result{};
    Metric<long> ms{ "some_metric_name" };
    ms.register_aggregator(avg)
        .register_aggregator(max)
        .register_aggregator(min)
        .register_aggregator(sum);

    for (auto v : values)
    {
        bool rec_result = ms.receive(v);
        EXPECT_TRUE(rec_result);
    }

    avg->get_val(avg_result);
    max->get_val(max_result);
    min->get_val(min_result);
    sum->get_val(sum_result);

    EXPECT_EQ(avg_result, 157 / values.size());
    EXPECT_EQ(max_result, 67);
    EXPECT_EQ(min_result, -10);
    EXPECT_EQ(sum_result, 157);
}
