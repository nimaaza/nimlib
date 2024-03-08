#pragma once

// #include <cstddef>

#include "timer.h"

namespace nimlib::Metrics::Aggregations
{
    struct Aggregator
    {
        virtual ~Aggregator() = default;

        virtual bool involve(long) = 0;
        virtual bool get_val(long&) = 0;
    };

    class Increment : public Aggregator
    {
    public:
        Increment();
        ~Increment();

        bool involve(long m = 1) override;
        bool get_val(long& m) override;

    private:
        long count;
    };

    class Sum : public Aggregator
    {
    public:
        Sum();
        ~Sum();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        long sum;
    };

    class Max : public Aggregator
    {
    public:
        Max();
        ~Max();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        long max;
    };

    class Min : public Aggregator
    {
    public:
        Min();
        ~Min();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        long min;
    };

    class Avg : public Aggregator
    {
    public:
        Avg();
        ~Avg();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        Increment increment;
        Sum sum;
        long avg;
    };

    class RatePerSecond : public Aggregator
    {
    public:
        RatePerSecond();
        ~RatePerSecond();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        nimlib::Metrics::Measurements::Timer timer{};
        Sum sum_of_time{};
        Sum sum_of_count{};
    };
};
