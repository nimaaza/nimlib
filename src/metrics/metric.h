#pragma once

#include <chrono>
#include <string>
#include <vector>
#include <algorithm>

#include "aggregations.h"
#include "../utils/circular_array.h"

namespace nimlib::Server::Metrics
{
    using nimlib::Sever::Utils::CircularArray;

    template <typename T>
    struct Metric
    {
        using aggregator_ptr = std::shared_ptr<Aggregations::Aggregator<T>>;
        using report_data_t = std::vector<std::pair<std::string, std::vector<T>>>;

        virtual ~Metric() = default;

        virtual Metric& register_aggregator(aggregator_ptr agg) = 0;
        virtual bool receive(T m) = 0;
        virtual const std::string& get_name() const = 0;
        virtual void get_report(report_data_t& aggregations) const = 0;
    };

    template <typename T>
    class PointMetric : public Metric<T>
    {
    public:
        using aggregator_ptr = std::shared_ptr<Aggregations::Aggregator<T>>;
        using report_data_t = std::vector<std::pair<std::string, std::vector<T>>>;

        explicit PointMetric(const std::string& name);
        ~PointMetric() = default;

        PointMetric(const PointMetric&) = delete;
        PointMetric& operator=(const PointMetric&) = delete;
        PointMetric(PointMetric&&) noexcept = delete;
        PointMetric& operator=(PointMetric&&) noexcept = delete;

    public:
        PointMetric& register_aggregator(aggregator_ptr agg) override;
        bool receive(T m) override;
        const std::string& get_name() const override;
        void get_report(report_data_t& aggregations) const override;

    private:
        std::string name;
        std::vector<aggregator_ptr> aggregators{};
    };

    template <typename T>
    class TimeSeriesMetric : public Metric<T>
    {
    public:
        using aggregator_ptr = std::shared_ptr<Aggregations::Aggregator<T>>;
        using report_data_t = std::vector<std::pair<std::string, std::vector<T>>>;

        TimeSeriesMetric(std::shared_ptr<Metric<T>> metric, int capacity);
        ~TimeSeriesMetric() = default;

        TimeSeriesMetric(const TimeSeriesMetric&) = delete;
        TimeSeriesMetric& operator=(const TimeSeriesMetric&) = delete;
        TimeSeriesMetric(TimeSeriesMetric&&) noexcept = delete;
        TimeSeriesMetric& operator=(TimeSeriesMetric&&) noexcept = delete;

    public:
        TimeSeriesMetric& register_aggregator(aggregator_ptr agg) override;
        bool receive(T m) override;
        const std::string& get_name() const override;
        void get_report(report_data_t& aggregations) const override;

    private:
        std::shared_ptr<Metric<T>> metric;
        int capacity;
        CircularArray<std::chrono::time_point<std::chrono::steady_clock>> ts;
        std::vector<CircularArray<T>> metric_data{};
    };
};

namespace nimlib::Server::Metrics
{
    template<typename T>
    PointMetric<T>::PointMetric(const std::string& name)
        : Metric<T>{}, name{ name }
    {}

    template<typename T>
    PointMetric<T>& PointMetric<T>::register_aggregator(aggregator_ptr agg)
    {
        if (agg) aggregators.push_back(agg);
        return *this;
    }

    template<typename T>
    bool PointMetric<T>::receive(T m)
    {
        bool well_received{ true };

        for (auto& agg : aggregators)
        {
            well_received = well_received && agg->involve(m);
        }

        return well_received;
    }

    template<typename T>
    const std::string& PointMetric<T>::get_name() const
    {
        return name;
    }

    template<typename T>
    void PointMetric<T>::get_report(report_data_t& aggregations) const
    {
        T val;
        for (auto& aggregator : aggregators)
        {
            aggregator->get_val(val);
            std::vector<T> v;
            v.push_back(val);
            aggregations.emplace_back(aggregator->get_name(), v);
        }
    }
};

namespace nimlib::Server::Metrics
{
    template <typename T>
    TimeSeriesMetric<T>::TimeSeriesMetric(
        std::shared_ptr<Metric<T>> metric,
        int capacity
    ) : metric{ metric }, capacity{ capacity }, ts{ capacity }
    {}

    template <typename T>
    TimeSeriesMetric<T>& TimeSeriesMetric<T>::register_aggregator(aggregator_ptr agg)
    {
        metric->register_aggregator(agg);
        return *this;
    }

    template <typename T>
    bool TimeSeriesMetric<T>::receive(T m)
    {
        bool receive_result = metric->receive(m);

        report_data_t report_data;
        metric->get_report(report_data);

        if (metric_data.size() != report_data.size())
        {
            metric_data.clear();
            for (int i = 0; i < report_data.size(); i++)
            {
                metric_data.emplace_back(capacity);
            }
        }

        for (int i = 0; i < report_data.size(); i++)
        {
            auto& val = report_data[i].second;
            metric_data[i].push_back(val[0]);
        }

        ts.push_back(std::chrono::steady_clock::now());

        return receive_result;
    }

    template <typename T>
    const std::string& TimeSeriesMetric<T>::get_name() const { return metric->get_name(); }

    template <typename T>
    void TimeSeriesMetric<T>::get_report(report_data_t& aggregations) const
    {
        report_data_t report_data;
        metric->get_report(report_data);
        for (int i = 0; i < report_data.size(); i++)
        {
            aggregations.emplace_back(report_data[i].first, metric_data[i].copy());
        }

        auto time_stamps = ts.copy();
        std::vector<T> time_stamps_seconds;
        time_stamps_seconds.resize(time_stamps.size());
        std::transform(
            time_stamps.begin(),
            time_stamps.end(),
            time_stamps_seconds.begin(),
            [](const auto& time_point) {
                return std::chrono::duration_cast<std::chrono::seconds>(time_point.time_since_epoch()).count();
            }
        );

        aggregations.emplace_back(std::string("time"), time_stamps_seconds);
    }
}
