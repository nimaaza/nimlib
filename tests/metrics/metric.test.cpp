#include <gtest/gtest.h>

#include <vector>
#include <numeric>

#include "../../src/metrics/metrics_store.h"
#include "../../src/metrics/aggregations.h"
#include "../../src/metrics/metric.h"

using namespace nimlib::Server::Metrics;

class MetricTestFixture : public testing::Test
{
public:
    MetricTestFixture() : metric{ "some_metric_name" }
    {
        metric.register_aggregator(avg)
            .register_aggregator(max)
            .register_aggregator(min)
            .register_aggregator(sum)
            .register_aggregator(med);
    };

protected:
    PointMetric<long> metric;
    std::shared_ptr<Aggregations::Avg<long>> avg = std::make_shared<Aggregations::Avg<long>>();
    std::shared_ptr<Aggregations::Max<long>> max = std::make_shared<Aggregations::Max<long>>();
    std::shared_ptr<Aggregations::Min<long>> min = std::make_shared<Aggregations::Min<long>>();
    std::shared_ptr<Aggregations::Sum<long>> sum = std::make_shared<Aggregations::Sum<long>>();
    std::shared_ptr<Aggregations::Med<long>> med = std::make_shared<Aggregations::Med<long>>();
    long avg_result{};
    long max_result{};
    long min_result{};
    long sum_result{};
    long med_result{};
};

TEST_F(MetricTestFixture, DifferentValues)
{
    std::vector<int> values{ 4, 6, 8, 12, 45, 67, 8, 0, 9, -1, -2, 4, 7, -10, 4 };

    bool rec_result = true;
    for (auto v : values)
    {
        rec_result = rec_result && metric.receive(v);
    }

    avg->get_val(avg_result);
    max->get_val(max_result);
    min->get_val(min_result);
    sum->get_val(sum_result);
    med->get_val(med_result);

    EXPECT_TRUE(rec_result);
    EXPECT_EQ(avg_result, std::accumulate(values.begin(), values.end(), 0) / values.size());
    EXPECT_EQ(max_result, 67);
    EXPECT_EQ(min_result, -10);
    EXPECT_EQ(sum_result, std::accumulate(values.begin(), values.end(), 0));
    EXPECT_EQ(med_result, 6);
}

TEST_F(MetricTestFixture, RepeatedValues)
{
    std::vector<int> values{ 7, 7, 7, 7, 7 };

    bool rec_result = true;
    for (auto v : values)
    {
        rec_result = rec_result && metric.receive(v);
    }

    avg->get_val(avg_result);
    max->get_val(max_result);
    min->get_val(min_result);
    sum->get_val(sum_result);
    med->get_val(med_result);

    EXPECT_TRUE(rec_result);
    EXPECT_EQ(avg_result, std::accumulate(values.begin(), values.end(), 0) / values.size());
    EXPECT_EQ(max_result, 7);
    EXPECT_EQ(min_result, 7);
    EXPECT_EQ(sum_result, std::accumulate(values.begin(), values.end(), 0));
    EXPECT_EQ(med_result, 7);
}
