#include "src/polling_server.h"
#include "src/metrics/factory.h"
#include "src/common/decorators.h"

int main()
{
    using nimlib::Server::PollingServer;
    using nimlib::Server::Decorators::decorate;
    using metrics_factory = nimlib::Server::Metrics::Factory<long>;

    metrics_factory::instantiate_metric(nimlib::Server::Constants::TIME_TO_RESPONSE)
        .measure_avg()
        .measure_max()
        .get();

    auto psl = decorate(std::make_unique<PollingServer>("8080"));
    psl->run();
}
