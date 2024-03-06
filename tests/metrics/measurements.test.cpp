#include <gtest/gtest.h>

#include "../../src/metrics/measurements.h"

using namespace nimlib::Metrics::Measurements;

TEST(CountMeasurement, _)
{
    const std::string m_name {"metric"};

    Count count {m_name};
}