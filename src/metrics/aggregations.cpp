#include "aggregations.h"

namespace nimlib::Metrics::Aggregations
{
    Increment::Increment() : count{} {}

    Increment::~Increment() {}

    bool Increment::involve(long m)
    {
        m > 0 ? count += 1 : count -= 1;
        return true;
    }

    bool Increment::get_val(long& m)
    {
        m = count;
        return true;
    }
}

namespace nimlib::Metrics::Aggregations
{
    Sum::Sum() : sum{} {}

    Sum::~Sum() {}

    bool Sum::involve(long m)
    {
        sum += m;
        return true;
    }

    bool Sum::get_val(long& m)
    {
        m = sum;
        return true;
    }
}

namespace nimlib::Metrics::Aggregations
{
    Max::Max() : max{ -1000000000 } {} // TODO: initialize with minus infinity

    Max::~Max() {}

    bool Max::involve(long m)
    {
        if (m > max) max = m;
        return true;
    }

    bool Max::get_val(long& m)
    {
        m = max;
        return true;
    }
}

namespace nimlib::Metrics::Aggregations
{
    Min::Min() : min{ 1000000000 } {} // TODO: initialize with infinity

    Min::~Min() {}

    bool Min::involve(long m)
    {
        if (m < min) min = m;
        return true;
    }

    bool Min::get_val(long& m)
    {
        m = min;
        return true;
    }
}

namespace nimlib::Metrics::Aggregations
{
    Avg::Avg() : avg{}, increment{}, sum{} {}

    Avg::~Avg() {}

    bool Avg::involve(long m)
    {
        increment.involve(1);
        sum.involve(m);
        return true;
    }

    bool Avg::get_val(long& m)
    {
        long s{};
        long c{};

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

namespace nimlib::Metrics::Aggregations
{
    RatePerSecond::RatePerSecond() { timer.begin(); }

    RatePerSecond::~RatePerSecond() {}

    bool RatePerSecond::involve(long m)
    {
        long latency;

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

    bool RatePerSecond::get_val(long& m)
    {
        long count{};
        long time{};

        bool value_collection_result = sum_of_count.get_val(count) && sum_of_time.get_val(time);
        if (value_collection_result)
        {
            m = count / time;
        }

        return value_collection_result;
    }
}
