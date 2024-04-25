#pragma once

#include "timer.h"

#include <limits>
#include <queue>
#include <vector>

namespace nimlib::Server::Metrics::Aggregations
{
    template<typename T>
    struct Aggregator
    {
        virtual ~Aggregator() = default;

        virtual bool involve(T) = 0;
        virtual bool get_val(T&) = 0;
    };
};

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    class Increment : public Aggregator<T>
    {
    public:
        Increment();
        ~Increment() = default;

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
        ~Sum() = default;

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
        ~Max() = default;

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
        ~Min() = default;

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
        ~Avg() = default;

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        Increment<T> increment;
        Sum<T> sum;
        T avg;
    };

    template <typename T>
    class Med : public Aggregator<T>
    {
    public:
        Med();
        ~Med() = default;

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        std::priority_queue<T, std::vector<T>, std::less<T>> less{};
        std::priority_queue<T, std::vector<T>, std::greater<T>> more{};
    };

    template <typename T>
    class AvgRatePerSecond : public Aggregator<T>
    {
    public:
        AvgRatePerSecond();
        ~AvgRatePerSecond() = default;

        bool involve(T m) override;
        bool get_val(T& m) override;

    private:
        nimlib::Server::Metrics::Measurements::Timer timer{};
        Sum<T> duration{};
        Sum<T> count{};
    };
};

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    Increment<T>::Increment() : count{} {}

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
    Max<T>::Max() : max{ std::numeric_limits<T>::min() } {}

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
    Min<T>::Min() : min{ std::numeric_limits<T>::max() } {}

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

        if (sum.get_val(s) && increment.get_val(c) && c != 0)
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
    Med<T>::Med()
    {
        less.push(std::numeric_limits<T>::min());
        more.push((std::numeric_limits<T>::max() - 1) * 1);
    }

    template <typename T>
    bool Med<T>::involve(T m)
    {
        m <= less.top() ? less.push(m) : more.push(m);

        auto count = less.size() + more.size();

        if (count % 2 == 0 && less.size() < more.size())
        {
            while (less.size() != more.size())
            {
                less.push(more.top());
                more.pop();
            }
        }
        else if (count % 2 == 0 && less.size() > more.size())
        {
            while (less.size() != more.size())
            {
                more.push(less.top());
                less.pop();
            }
        }
        else if (count % 2 != 0 && less.size() < more.size() + 1)
        {
            while (less.size() != more.size() + 1)
            {
                less.push(more.top());
                more.pop();
            }
        }
        else if (count % 2 != 0 && less.size() > more.size() + 1)
        {
            while (less.size() != more.size() + 1)
            {
                more.push(less.top());
                less.pop();
            }
        }

        return true;
    }

    template<typename T>
    bool Med<T>::get_val(T& m)
    {
        m = less.top();
        return true;
    }
}

namespace nimlib::Server::Metrics::Aggregations
{
    template <typename T>
    AvgRatePerSecond<T>::AvgRatePerSecond() { timer.begin(); }

    template <typename T>
    bool AvgRatePerSecond<T>::involve(T m)
    {
        T latency;

        if (timer.end(latency))
        {
            latency /= 1'000'000'000; // Converting to seconds.
            bool value_collection_result = duration.involve(latency) && count.involve(m);
            timer.begin();
            return value_collection_result;
        }
        else
        {
            return false;
        }
    }

    template <typename T>
    bool AvgRatePerSecond<T>::get_val(T& m)
    {
        T c{};
        T t{};

        bool value_collection_result = count.get_val(c) && duration.get_val(t);
        if (value_collection_result)
        {
            m = c / t;
        }

        return value_collection_result;
    }
}
