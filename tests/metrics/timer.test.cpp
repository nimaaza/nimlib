#include <gtest/gtest.h>

#include <time.h>

#include "../../src/metrics/timer.h"

using namespace nimlib::Server::Metrics::Measurements;

TEST(TimerTest, ProperUse)
{
    timespec t1, t2;
    t1.tv_sec = 1L;
    t1.tv_nsec = 100'000'000L;
    long duration;
    Timer timer{};

    bool begin_success = timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // Failed to sleep for some reason and this will
        // cause the test to fail.
    }
    bool end_success = timer.end(duration);

    EXPECT_GE(duration, 1'100'000'000);
    EXPECT_TRUE(begin_success);
    EXPECT_TRUE(end_success);
}

TEST(TimerTest, IncorrectUsage_CallEndBeforeBegin)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 100L;
    long duration{ 134 }; // Just a random value (not significant).
    Timer timer{};

    // timer.begin() not called by mistake.
    // timer.begin()
    if (nanosleep(&t1, &t2) < 0)
    {
        // Failed to sleep for some reason and this will
        // cause the test to fail.
    }
    bool end_success = timer.end(duration);

    EXPECT_EQ(duration, 134); // duration is not modified.
    EXPECT_FALSE(end_success);
}

TEST(TimerTest, IncorrectUsage_CallBeginTwice)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 1000L;
    long duration{};
    Timer timer{};

    // The first call to begin() starts the timer. The
    // second call has no effect on the timer and doesn't
    // also stop it.
    bool begin_success_1 = timer.begin();
    bool begin_success_2 = timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // Failed to sleep for some reason and this will
        // cause the test to fail.
    }
    bool end_success = timer.end(duration);

    EXPECT_GE(duration, 1000);
    EXPECT_TRUE(begin_success_1);
    EXPECT_FALSE(begin_success_2);
    EXPECT_TRUE(end_success);
}

TEST(TimerTest, IncorrectUsage_CallEndTwice)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 1000L;
    long duration_1{ 34 }; // Just a random value (not significant).
    long duration_2{ 34 };
    Timer timer{};

    // The first call to timer.end() is correct and ends the timing so
    // duration_1 is modified and the duration measurement is written
    // to it. The second call to timer.end() has no effect.
    bool begin_success = timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // Failed to sleep for some reason and this will
        // cause the test to fail.
    }
    bool end_success_1 = timer.end(duration_1);
    bool end_success_2 = timer.end(duration_2);

    EXPECT_NE(duration_1, 1000);
    EXPECT_TRUE(begin_success);
    EXPECT_TRUE(end_success_1);
    EXPECT_EQ(duration_2, 34);
    EXPECT_FALSE(end_success_2);
}

TEST(TimerTest, UsableAfterIncorrectUsage)
{
    timespec t1, t2;
    t1.tv_sec = 0L;
    t1.tv_nsec = 1'000L;
    long duration_1{ 134 }; // Just random values.
    long duration_2{ 56 };
    Timer timer{};

    // timer.begin() not called by mistake.
    // timer.begin()
    if (nanosleep(&t1, &t2) < 0)
    {
        // Failed to sleep for some reason and this will
        // cause the test to fail.
    }
    bool success_1 = timer.end(duration_1);

    timer.begin();
    if (nanosleep(&t1, &t2) < 0)
    {
        // Failed to sleep for some reason and this will
        // cause the test to fail.
    }
    bool success_2 = timer.end(duration_2);

    EXPECT_EQ(duration_1, 134); // duration_1 is not modified.
    EXPECT_FALSE(success_1);
    EXPECT_GE(duration_2, 1000);
    EXPECT_TRUE(success_2);
}
