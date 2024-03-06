#pragma once

// #include <cstddef>

#include "timer.h"

namespace nimlib::Metrics::Aggregations
{
    struct Opt
    {
        virtual ~Opt() = default;

        virtual bool involve(long) = 0;
        virtual bool get_val(long&) = 0;
    };

    class Increment : public Opt
    {
    public:
        Increment();
        ~Increment();

        bool involve(long m = 1) override;
        bool get_val(long& m) override;

    private:
        long count;
    };

    class Sum : public Opt
    {
    public:
        Sum();
        ~Sum();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        long sum;
    };

    class Max : public Opt
    {
    public:
        Max();
        ~Max();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        long max;
    };

    class Min : public Opt
    {
    public:
        Min();
        ~Min();

        bool involve(long m) override;
        bool get_val(long& m) override;

    private:
        long min;
    };

    class Avg : public Opt
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

    class RatePerSecond : public Opt
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
