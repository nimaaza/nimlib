#include "src/polling_server.h"
#include "src/metrics/builder.h"
#include "src/common/decorators.h"

int main()
{
    using nimlib::Server::PollingServer;
    using nimlib::Server::Decorators::decorate;
    using metrics_builder = nimlib::Server::Metrics::Builder<long>;

    metrics_builder::instantiate_metric(nimlib::Server::Constants::TIME_TO_RESPONSE)
        .measure_avg()
        .measure_max()
        .measure_avg_rate()
        .measure_med()
        .with_timeseries(10)
        .build();

    auto psl = decorate(std::make_unique<PollingServer>("8080"));
    psl->run();
}
