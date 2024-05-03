#include <gtest/gtest.h>

#include "../../src/utils/circular_array.h"

using nimlib::Sever::Utils::CircularArray;

TEST(CircularArray, CapacityOfOne)
{
    CircularArray<int> ca{ 1 };

    ca.push_back(1);
    ca.push_back(2);
    ca.push_back(3);
    auto actual = ca.copy();
    auto expected = std::vector<int>{};
    expected.push_back(3);

    EXPECT_EQ(actual, expected);
    EXPECT_EQ(ca[0], 3);
}

TEST(CircularArray, UsedLessThanCapacity)
{
    CircularArray<int> ca{ 5 };

    for (int i = 1; i <= 3; i++)
    {
        ca.push_back(i);
    }
    auto actual = ca.copy();
    auto expected = std::vector<int>{ 1, 2, 3 };

    EXPECT_EQ(actual, expected);
    EXPECT_EQ(ca[0], 1);
    EXPECT_EQ(ca[1], 2);
    EXPECT_EQ(ca[2], 3);
}

TEST(CircularArray, UsedMoreThanCapacity)
{
    CircularArray<int> ca{ 5 };

    for (int i = 0; i <= 6; i++)
    {
        ca.push_back(i);
    }
    auto actual = ca.copy();
    auto expected = std::vector<int>{ 2, 3, 4, 5, 6 };

    EXPECT_EQ(actual, expected);
    EXPECT_EQ(ca[0], 2);
    EXPECT_EQ(ca[1], 3);
    EXPECT_EQ(ca[2], 4);
    EXPECT_EQ(ca[3], 5);
    EXPECT_EQ(ca[4], 6);
}

TEST(CircularArray, UsedMuchMoreThanCapacity)
{
    CircularArray<int> ca{ 4 };

    for (int i = 0; i <= 17; i++)
    {
        ca.push_back(i);
    }
    auto actual = ca.copy();
    auto expected = std::vector<int>{ 14, 15, 16, 17 };

    EXPECT_EQ(actual, expected);
    EXPECT_EQ(ca[0], 14);
    EXPECT_EQ(ca[1], 15);
    EXPECT_EQ(ca[2], 16);
    EXPECT_EQ(ca[3], 17);
}