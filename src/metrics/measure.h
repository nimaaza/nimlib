#pragma once

#include "metrics_store.h"

#include <string>
#include <memory>

namespace nimlib::Server::Metrics::Measurements
{
    template<typename T>
    struct Measure
    {
        explicit Measure(const std::string& target_name)
            : metric{ nimlib::Server::Metrics::MetricsStore<T>::get_instance().get_metric(target_name) }
        {};
        virtual ~Measure() = default;

        // TODO: copy & move constructors for base class?

        virtual void start() = 0;
        virtual void end() = 0;
        virtual void cancel() = 0;

    protected:
        void forward_measurement(T m)
        {
            if (metric) metric->receive(m);
        }

    protected:
        std::shared_ptr<nimlib::Server::Metrics::Metric<T>> metric;
    };
};

namespace nimlib::Server::Metrics::Measurements
{
    template<typename T>
    class Count : public Measure<T>
    {
    public:
        explicit Count(const std::string& target_name);
        ~Count() = default;

        void start() override;
        void end() override;
        void cancel() override;
    };

    template<typename T>
    class Duration : public Measure<T>
    {
    public:
        explicit Duration(const std::string& target_name);
        ~Duration() = default;

        void start() override;
        void end() override;
        void cancel() override;

    private:
        nimlib::Server::Utils::Timer timer{};
    };
};

namespace nimlib::Server::Metrics::Measurements
{
    template<typename T>
    Count<T>::Count(const std::string& target_name) : Measure<T>{ target_name } {}

    template<typename T>
    void Count<T>::start() {}

    template<typename T>
    void Count<T>::end() { Measure<T>::forward_measurement(1); }

    template<typename T>
    void Count<T>::cancel() {}
}

namespace nimlib::Server::Metrics::Measurements
{
    template<typename T>
    Duration<T>::Duration(const std::string& target_name) : Measure<T>{ target_name } {}

    template<typename T>
    void Duration<T>::start() { timer.begin(); }

    template<typename T>
    void Duration<T>::end()
    {
        long duration;
        if (timer.end(duration))
        {
            Measure<T>::forward_measurement(duration);
        }
    }

    template<typename T>
    void Duration<T>::cancel()
    {
        long duration;
        timer.end(duration);
    }
}
