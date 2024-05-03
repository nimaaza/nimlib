#include "http_router.h"

#include "../metrics/metrics_store.h"

#include <fstream>
#include <iostream>
#include <iomanip>

namespace nimlib::Server::Handlers
{
    HttpRouter::HttpRouter() = default;

    HttpRouter::~HttpRouter() = default;

    HttpResponse HttpRouter::route(HttpRequest& http_request)
    {
        if (http_request.target == "/metrics")
        {
            auto& metrics_store = nimlib::Server::Metrics::MetricsStore<long>::get_instance();
            auto report = metrics_store.generate_stats_report();

            HttpResponse http_response{};
            http_response.status = 200;
            http_response.reason = "OK";
            http_response.headers["content-type"].push_back("text/plain");
            http_response.body = report;
            return http_response;
        }
        else if (http_request.target == "/files1/img.jpg")
        {
            std::string contents;
            std::ifstream in("./img.jpg", std::ios::in | std::ios::binary);
            if (in)
            {
                in.seekg(0, std::ios::end);
                contents.resize(in.tellg());
                in.seekg(0, std::ios::beg);
                in.read(&contents[0], contents.size());
                in.close();
            }

            HttpResponse http_response{};
            http_response.status = 200;
            http_response.reason = "OK";
            http_response.headers["content-type"].push_back("image/jpeg");
            http_response.body = contents;
            return http_response;
        }
        else if (http_request.target == "/login" && http_request.method == "POST")
        {
            std::cout << http_request.body << std::endl;
            HttpResponse http_response{};
            http_response.status = 404;
            http_response.reason = "Not found";
            http_response.headers["content-type"].push_back("text/html; charset=UTF-8");
            http_response.body = "not found";
            return http_response;
        }
        else
        {
            HttpResponse http_response{};
            http_response.status = 404;
            http_response.reason = "Not found";
            http_response.headers["content-type"].push_back("text/html; charset=UTF-8");
            http_response.body = "not found";
            return http_response;
        }
    }
}
