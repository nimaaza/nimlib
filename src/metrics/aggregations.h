#pragma once

// #include <cstddef>

#include "timer.h"

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    struct Aggregator
    {
        virtual ~Aggregator() = default;

        virtual bool involve(T) = 0;
        virtual bool get_val(T&) = 0;
    };

    template <typename T>
    class Increment : public Aggregator<T>
    {
    public:
        Increment();
        ~Increment();

        bool involve(T m = 1) override;
        bool get_val(T& m) override;

    private:
        T count;
    };

    template <typename T>
    class Sum : public Aggregator<T>
    {
    public:
        Sum();
        ~Sum();

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        T sum;
    };

    template <typename T>
    class Max : public Aggregator<T>
    {
    public:
        Max();
        ~Max();

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        T max;
    };

    template <typename T>
    class Min : public Aggregator<T>
    {
    public:
        Min();
        ~Min();

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        T min;
    };

    template <typename T>
    class Avg : public Aggregator<T>
    {
    public:
        Avg();
        ~Avg();

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        Increment<T> increment;
        Sum<T> sum;
        T avg;
    };

    template <typename T>
    class RatePerSecond : public Aggregator<T>
    {
    public:
        RatePerSecond();
        ~RatePerSecond();

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        nimlib::Server::Metrics::Measurements::Timer timer{};
        Sum<T> sum_of_time{};
        Sum<T> sum_of_count{};
    };
};

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    Increment<T>::Increment() : count{} {}

    template <typename T>
    Increment<T>::~Increment() {}

    template <typename T>
    bool Increment<T>::involve(T m)
    {
        m > 0 ? count += 1 : count -= 1;
        return true;
    }

    template <typename T>
    bool Increment<T>::get_val(T& m)
    {
        m = count;
        return true;
    }
}

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    Sum<T>::Sum() : sum{} {}

    template <typename T>
    Sum<T>::~Sum() {}

    template <typename T>
    bool Sum<T>::involve(T m)
    {
        sum += m;
        return true;
    }

    template <typename T>
    bool Sum<T>::get_val(T& m)
    {
        m = sum;
        return true;
    }
}

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    Max<T>::Max() : max{ -1000000000 } {} // TODO: initialize with minus infinity

    template <typename T>
    Max<T>::~Max() {}

    template <typename T>
    bool Max<T>::involve(T m)
    {
        if (m > max) max = m;
        return true;
    }

    template <typename T>
    bool Max<T>::get_val(T& m)
    {
        m = max;
        return true;
    }
}

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    Min<T>::Min() : min{ 1000000000 } {} // TODO: initialize with infinity

    template <typename T>
    Min<T>::~Min() {}

    template <typename T>
    bool Min<T>::involve(T m)
    {
        if (m < min) min = m;
        return true;
    }

    template <typename T>
    bool Min<T>::get_val(T& m)
    {
        m = min;
        return true;
    }
}

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    Avg<T>::Avg() : avg{}, increment{}, sum{} {}

    template <typename T>
    Avg<T>::~Avg() {}

    template <typename T>
    bool Avg<T>::involve(T m)
    {
        increment.involve(1);
        sum.involve(m);
        return true;
    }

    template <typename T>
    bool Avg<T>::get_val(T& m)
    {
        T s{};
        T c{};

        if (sum.get_val(s) && increment.get_val(c))
        {
            m = s / c;

            return true;
        }
        else
        {
            return false;
        }
    }
}

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    RatePerSecond<T>::RatePerSecond() { timer.begin(); }

    template <typename T>
    RatePerSecond<T>::~RatePerSecond() {}

    template <typename T>
    bool RatePerSecond<T>::involve(T m)
    {
        T latency;

        if (timer.end(latency))
        {
            latency /= 1'000'000'000; // Converting to seconds.
            bool value_collection_result = sum_of_time.involve(latency) && sum_of_count.involve(m);
            timer.begin();
            return value_collection_result;
        }
        else
        {
            return false;
        }
    }

    template <typename T>
    bool RatePerSecond<T>::get_val(T& m)
    {
        T count{};
        T time{};

        bool value_collection_result = sum_of_count.get_val(count) && sum_of_time.get_val(time);
        if (value_collection_result)
        {
            m = count / time;
        }

        return value_collection_result;
    }
}
