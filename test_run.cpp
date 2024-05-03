#include "src/polling_server.h"
#include "src/metrics/builder.h"
#include "src/common/decorators.h"
#include <functional>
#include "src/http/http_parser.h"

//using nimlib::Server::Handlers::HttpRequest;
//using nimlib::Server::Handlers::HttpResponse;
//
//enum Method {GET, POST};
//
//template <typename T>
//struct Routing
//{
//    T method;
//    const std::string target;
//    const std::unordered_map<std::string, std::unique_ptr<Routing>> next_targets;
//    std::function<void (const HttpRequest&, HttpResponse&)> handler;
//};
//
//struct Router
//{
//    Router() = default;
//    ~Router() = default;
//
//    void add(std::string method, std::string target, std::function<void (const HttpRequest&, HttpResponse&)>) {};
//};

int main()
{
    using nimlib::Server::PollingServer;
    using nimlib::Server::Decorators::decorate;
    using metrics_builder = nimlib::Server::Metrics::Builder<long>;

    //    Router router {};
    //    router.add("get", "/file/img.jpg", [](const HttpRequest& request, HttpResponse& response) { return; });

    metrics_builder::instantiate_metric(nimlib::Server::Constants::TIME_TO_RESPONSE)
        .measure_avg()
        .measure_max()
        .with_timeseries(5)
        .build();

    auto psl = decorate(std::make_unique<PollingServer>("8080"));
    psl->run();
}
