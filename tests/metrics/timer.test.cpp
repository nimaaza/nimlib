#include <gtest/gtest.h>

#include <time.h>

#include "../../src/metrics/timer.h"

using namespace nimlib::Server::Metrics::Measurements;

TEST(TimerTest, ProperUse)
{
    timespec t1, t2;
    t1.tv_sec = 1L;
    t1.tv_nsec = 100'000'000L;
    long latency;
    Timer timer{};

    bool begin_success = timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }
    bool end_success = timer.end(latency);

    EXPECT_GE(latency, 1'100'000'000);
    EXPECT_TRUE(begin_success);
    EXPECT_TRUE(end_success);
}

TEST(TimerTest, IncorrectUsage_CallEndBeforeBegin)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 100L;
    long latency{ 134 }; // Just a random value (not significant).
    Timer timer{};

    // timer.begin() not called by mistake
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }
    bool end_succdss = timer.end(latency);

    EXPECT_GE(latency, 134); // latency is not modified.
    EXPECT_FALSE(end_succdss);
}

TEST(TimerTest, IncorrectUsage_CallBeginTwice)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 100L;
    long latency{ 134 }; // Just a random value (not significant).
    Timer timer{};

    bool begin_success_1 = timer.begin();
    bool begin_success_2 = timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }
    bool end_success = timer.end(latency);

    EXPECT_GE(latency, 134); // latency is not modified.
    EXPECT_TRUE(begin_success_1);
    EXPECT_FALSE(begin_success_2);
    EXPECT_TRUE(end_success);
}

TEST(TimerTest, IncorrectUsage_CallEndTwice)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 100L;
    long latency_1{ 34 }; // Just a random value (not significant).
    long latency_2{ 34 };
    Timer timer{};

    bool begin_success = timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }
    bool end_success_1 = timer.end(latency_1);
    bool end_success_2 = timer.end(latency_2);

    EXPECT_NE(latency_1, 34); // latency is not modified.
    EXPECT_TRUE(begin_success);
    EXPECT_TRUE(end_success_1);
    EXPECT_EQ(latency_2, 34);
    EXPECT_FALSE(end_success_2);
}

TEST(TimerTest, UsableAfterIncorrectUsage)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 1'000L;
    long latency_1{ 134 }; // Just random values.
    long latency_2{ 56 };
    Timer timer{};

    // timer.begin() not called by mistake
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }
    bool success_1 = timer.end(latency_1);

    timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // TODO: failed to sleep for some reason and will cause the test to fail
    }
    bool success_2 = timer.end(latency_2);

    EXPECT_EQ(latency_1, 134); // latency_1 is not modified.
    EXPECT_FALSE(success_1);
    EXPECT_NE(latency_2, 56);
    EXPECT_TRUE(success_2);
}
