#include <gtest/gtest.h>

#include "../../src/metrics/measurements.h"

using namespace nimlib::Server::Metrics::Measurements;

TEST(CountMeasurement, _)
{
    const std::string m_name {"metric"};

    Count<long> count {m_name};
}