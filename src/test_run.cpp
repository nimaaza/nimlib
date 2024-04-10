#include <iostream>

#include "polling_server.h"
#include "metrics/factory.h"
#include "common/decorators.h"

int main()
{
    using nimlib::Server::PollingServer;
    using nimlib::Server::Decorators::decorate;
    using metrics_factory = nimlib::Server::Metrics::Factory<long>;

    metrics_factory::instanciate_metric(nimlib::Server::Constants::TIME_TO_RESPONSE)
        .measure_avg()
        .measure_max()
        .get();

    auto psl = decorate(std::make_unique<PollingServer>("8080"));
    psl->run();
}
