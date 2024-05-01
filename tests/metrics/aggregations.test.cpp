#include <gtest/gtest.h>

#include <algorithm>
#include <chrono>
#include <thread>
#include <numeric>
#include <vector>
#include <cstdlib>
#include <ctime>

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

    // Use default value for increment.
    if (increment.involve()) expected_count += 1;
    increment.get_val(actual_count);

    EXPECT_EQ(expected_count, actual_count);
}

TEST(MaxTest, ReturnsMaxSingleInput)
{
    Max<long> max{};
    long actual_max;

    max.involve(3);
    max.get_val(actual_max);

    EXPECT_EQ(3, actual_max);
}

TEST(MaxTest, ReturnsMax)
{
    std::vector<int> ints{ 1, 3, 7, 4, 23, 5, 6 , 7 };
    long expected_max{};
    long actual_max{ ints[0] };
    Max<long> max{};

    for (auto i : ints)
    {
        max.involve(i);
        if (i > expected_max) expected_max = i;
    }
    max.get_val(actual_max);

    EXPECT_EQ(expected_max, actual_max);
}

TEST(MaxTest, ReturnsMaxWithNegativeValues)
{
    std::vector<int> ints{ 1, -3, 7, 4, -23, 5, 0 , 7 };
    long expected_max{};
    long actual_max{ ints[0] };
    Max<long> max{};

    for (auto i : ints)
    {
        max.involve(i);
        if (i > expected_max) expected_max = i;
    }
    max.get_val(actual_max);

    EXPECT_EQ(expected_max, actual_max);
}

TEST(MinTest, ReturnsMinSingleInput)
{
    Min<long> min{};
    long actual_min;

    min.involve(3);
    min.get_val(actual_min);

    EXPECT_EQ(3, actual_min);
}

TEST(MinTest, ReturnsMin)
{
    std::vector<int> ints{ 12, 3, 7, 4, 23, 5, 6 , 7 };
    long expected_min{ ints[0] };
    long actual_min{  };
    Min<long> min{ };

    for (auto i : ints)
    {
        min.involve(i);
        if (i < expected_min) expected_min = i;
    }
    min.get_val(actual_min);

    EXPECT_EQ(expected_min, actual_min);
}

TEST(MinTest, ReturnsMinWithNegativeValues)
{
    std::vector<int> ints{ 12, -3, 7, 4, -23, 5, 0, 6 , -7, 131 };
    long expected_min{ ints[0] };
    long actual_min{  };
    Min<long> min{ };

    for (auto i : ints)
    {
        min.involve(i);
        if (i < expected_min) expected_min = i;
    }
    min.get_val(actual_min);

    EXPECT_EQ(expected_min, actual_min);
}

TEST(AvgTest, RetunsAvgSingleInput)
{
    long actual_avg{};
    Avg<long> avg{};

    avg.involve(6);
    avg.get_val(actual_avg);

    EXPECT_EQ(6, actual_avg);
}

TEST(AvgTest, RetunsAvg)
{
    std::vector<long> ints{ 3, 4, 6, 12, 34, 5, 7 };
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
    expected_avg /= (long)ints.size();
    avg.get_val(actual_avg);

    EXPECT_EQ(expected_avg, actual_avg);
}

TEST(AvgTest, RetunsAvgFloatingValue)
{
    std::vector<float> ints{ 3.1, 4.5, -6.01, 0, 12, -34, 5.001, 7.0, -9.87 };
    float expected_avg{};
    float actual_avg{};
    Avg<float> avg{};

    for (auto f : ints)
    {
        if (avg.involve(f))
        {
            expected_avg += f;
        }
    }
    expected_avg /= (float)ints.size();
    avg.get_val(actual_avg);

    EXPECT_LT(std::abs(expected_avg - actual_avg), 0.000'000'1);
}

TEST(AvgTest, RetunsAvgWithNegativeValues)
{
    std::vector<long> ints{ 3, 4, -6, 12, 34, 5, -7, 0, -78 };
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
    expected_avg /= (long)ints.size();
    avg.get_val(actual_avg);

    EXPECT_EQ(expected_avg, actual_avg);
}

TEST(AvgRatePerSecond, OverALongPeriod)
{
    AvgRatePerSecond<long> avg_rate{};
    long actual_rate{};
    long expected_rate{};
    std::srand(std::time(nullptr));
    std::vector<int> counts{ 100 };

    auto then = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000; i++)
    {
        auto count = std::rand() % 1000;
        counts.push_back(count);
        std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 10));
        avg_rate.involve(count);
    }
    auto now = std::chrono::steady_clock::now();
    bool success = avg_rate.get_val(actual_rate);

    auto count = std::accumulate(counts.begin(), counts.end(), 0);
    auto delay = std::chrono::duration_cast<std::chrono::seconds>(now - then).count();
    expected_rate = count / delay;
    auto div = std::abs((double)actual_rate - (double)expected_rate) * 100 / (double)expected_rate;

    EXPECT_TRUE(success);
    EXPECT_LT(div, 1.0);
}

TEST(AvgRatePerSecond, OverAShortPeriod)
{
    // For this aggregator the data makes sense over longer
    // durations (over 1 second).
    AvgRatePerSecond<long> avg_rate{};
    std::srand(std::time(nullptr));
    long actual_rate{};

    auto count = std::rand() % 1000;
    std::this_thread::sleep_for(std::chrono::milliseconds(std::rand() % 100));
    avg_rate.involve(count);
    bool success = avg_rate.get_val(actual_rate);

    EXPECT_FALSE(success);
}

TEST(MedianTest, SingleInput)
{
    int expected_median;
    Med<int> med{};

    med.involve(5);
    med.get_val(expected_median);

    EXPECT_EQ(expected_median, 5);
}

TEST(MedianTest, SimpleExample)
{
    std::vector<int> ints = { 1, 2, 3, 4, 5 };
    int expected_median;
    Med<int> med{};

    for (auto i : ints) med.involve(i);
    med.get_val(expected_median);

    EXPECT_EQ(expected_median, 3);
}

TEST(MedianTest, AllNegativeValues)
{
    std::vector<int> ints = { -4, -5, -3, -1, -2 };
    int expected_median;
    Med<int> med{};

    for (auto i : ints) med.involve(i);
    med.get_val(expected_median);

    EXPECT_EQ(expected_median, -3);
}

TEST(MedianTest, MedianOfOddCount)
{
    std::vector<int> ints = { 17, 4, 5, 10, -1, 6, 33 };
    int expected_median;
    Med<int> med{};

    for (auto i : ints) med.involve(i);
    med.get_val(expected_median);

    EXPECT_EQ(expected_median, 6);
}

TEST(MedianTest, MedianOfEvenCount)
{
    std::vector<int> ints = { 17, 4, 5, 10, -1, 6, 33, -7 };
    // -7, -1, 4, 5, 6, 10, 17, 33
    int expected_median;
    Med<int> med{};

    for (auto i : ints) med.involve(i);
    med.get_val(expected_median);

    EXPECT_EQ(expected_median, 5);
}

TEST(MedianTest, LargeArray)
{
    std::srand(std::time(nullptr));
    std::vector<long> ints;
    int size = 50'003;
    ints.resize(size);
    bool result{ true };
    long expected_median;
    long actual_median;
    Med<long> med{};

    for (int i = 0; i < size; i++)
    {
        auto rand_int = std::rand() * (std::rand() % 2 == 0 ? 1 : -1);
        result = result && med.involve(rand_int);
        ints[i] = rand_int;
    }
    std::sort(ints.begin(), ints.end());
    med.get_val(actual_median);
    expected_median = ints[(size - 1) / 2];

    EXPECT_TRUE(result);
    EXPECT_EQ(actual_median, expected_median);
}
