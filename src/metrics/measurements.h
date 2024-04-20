#pragma once

#include "metrics_store.h"

#include <string>
#include <memory>

namespace nimlib::Server::Metrics::Measurements
{
    template<typename T>
    struct Measurement
    {
        Measurement(const std::string& target_name)
            : metric{ nimlib::Server::Metrics::MetricsStore<T>::get_instance().get_metric(target_name) }
        {};

        virtual ~Measurement()
        {
            if (successful_measurement && metric)
            {
                metric->receive(measurement_result);
            }
        };

        // copy & move constructors?

        virtual void start() = 0;
        virtual void end() = 0;

    protected:
        std::shared_ptr<nimlib::Server::Metrics::Metric<T>> metric;
        T measurement_result{};
        bool successful_measurement{ false };
    };

    template<typename T>
    class Count : public Measurement<T>
    {
    public:
        Count(const std::string& target_name);
        ~Count();

        void start() override;
        void end() override;
    };

    template<typename T>
    class Duration : public Measurement<T>
    {
    public:
        Duration(const std::string& target_name);
        ~Duration();

        void start() override;
        void end() override;

    private:
        Timer timer{};
    };
};

namespace nimlib::Server::Metrics::Measurements
{
    template<typename T>
    Count<T>::Count(const std::string& target_name) : Measurement<T>{ target_name } {}

    template<typename T>
    Count<T>::~Count() {}

    template<typename T>
    void Count<T>::start() {}

    template<typename T>
    void Count<T>::end()
    {
        Measurement<T>::measurement_result = 1;
        Measurement<T>::successful_measurement = true;
    }

    template<typename T>
    Duration<T>::Duration(const std::string& target_name) : Measurement<T>{ target_name } {}

    template<typename T>
    Duration<T>::~Duration() {}

    template<typename T>
    void Duration<T>::start()
    {
        timer.begin();
    }

    template<typename T>
    void Duration<T>::end()
    {
        long duration;
        if (timer.end(duration))
        {
            Measurement<T>::measurement_result = duration;
            Measurement<T>::successful_measurement = true;
        }
    }
}
