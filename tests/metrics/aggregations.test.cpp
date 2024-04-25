#include <gtest/gtest.h>

#include <algorithm>
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

TEST(MedianTest, SingleValue)
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
