#include <gtest/gtest.h>

#include <vector>

#include "../../src/metrics/aggregations.h"

using namespace nimlib::Server::Metrics::Aggregations;

TEST(IncrementTest, IncrementsCorrectly)
{
    std::vector<int> ints{ 1, 2, -5, 7 };
    long expected_count{};
    long actual_count{};
    Increment<long> increment{};

    for (auto i : ints)
    {
        if (increment.involve(i))
            i > 0 ? expected_count += 1 : expected_count -= 1;
    }

    // Use default value for increment
    if (increment.involve()) expected_count += 1;

    increment.get_val(actual_count);
    EXPECT_EQ(expected_count, actual_count);
}

TEST(MaxTest, ReturnsMax)
{
    std::vector<int> ints{ 1, 3, 7, 4, 23, 5, 6 , 7 };
    long expected_max{};
    long actual_max{ ints[0] };
    Max<long> max{};

    for (auto i : ints)
    {
        if (max.involve(i) && i > expected_max) expected_max = i;
    }

    max.get_val(actual_max);
    EXPECT_EQ(expected_max, actual_max);
}

TEST(MinTest, ReturnsMin)
{
    std::vector<int> ints{ 12, 3, 7, 4, 23, 5, 6 , 7 };
    long expected_min{ 1000 };
    long actual_min{  };
    Min<long> min{ };

    for (auto i : ints)
    {
        if (min.involve(i) && i < expected_min) expected_min = i;
    }

    min.get_val(actual_min);
    EXPECT_EQ(expected_min, actual_min);
}

TEST(AvgTest, RetunsAvgRoundedDown)
{
    std::vector<int> ints{ 3, 4, 6, 12, 34, 5, 7 };
    long expected_avg{};
    long actual_avg{};
    Avg<long> avg{};

    for (auto i : ints)
    {
        if (avg.involve(i))
        {
            expected_avg += i;
        }
    }
    expected_avg /= ints.size();

    avg.get_val(actual_avg);
    EXPECT_EQ(expected_avg, actual_avg);
}
