#include "router.h"

namespace nimlib::Server::Handlers::Http
{
    Router::Router()
    {
        Node get_node;
        Node post_node;
        http_method_handlers["GET"] = std::move(get_node);
        http_method_handlers["POST"] = std::move(post_node);
    }

    void Router::get(std::string target, route_handler handler)
    {
        add("GET", target, handler);
    }

    void Router::get(route_handler not_found_handler)
    {
        add_not_found("GET", not_found_handler);
    }

    void Router::post(std::string target, route_handler handler)
    {
        add("POST", target, handler);
    }

    void Router::post(route_handler not_found_handler)
    {
        add_not_found("POST", not_found_handler);
    }

    void Router::get(std::string target, const Request& request, Response& response)
    {
        handle("GET", target, request, response);
    }

    void Router::post(std::string target, const Request& request, Response& response)
    {
        handle("POST", target, request, response);
    }

    void Router::add(std::string method, std::string target, route_handler handler)
    {
        auto it = http_method_handlers.find(method);
        it->second.add(target, handler);
    }

    void Router::add_not_found(std::string method, route_handler handler)
    {
        auto it = http_method_handlers.find(method);
        it->second.not_found_handler = handler;
    }

    void Router::handle(std::string method, std::string target, const Request& request, Response& response)
    {
        auto it = http_method_handlers.find(method);
        params_t params;
        auto handler = it->second.find(target, params);
        if (handler)
        {
            handler.value()(request, response, params);
        }
    }

    route_handler Router::Node::not_found_handler = [](const Request&, Response&, params_t&) -> void {};

    Router::Node::Node(std::string target, route_handler h) { add(target, h); }

    void Router::Node::add(std::string target, route_handler h)
    {
        if (target.empty())
        {
            handler = h;
        }
        else
        {
            size_t n = target.find('/');
            std::string current_target_segment = target.substr(0, n);
            std::string rest_segment = (n == std::string::npos) ? "" : target.substr(n + 1);

            if (current_target_segment.starts_with('<') && current_target_segment.ends_with('>'))
            {
                parameter = current_target_segment.substr(1, current_target_segment.size() - 2);
                current_target_segment = parameter;
            }

            if (auto it = next_fixed_node.find(current_target_segment); it == next_fixed_node.end())
            {
                next_fixed_node.insert_or_assign(current_target_segment, Node(rest_segment, h));
            }
            else
            {
                it->second.add(rest_segment, h);
            }
        }
    }

    std::optional<route_handler> Router::Node::find(std::string_view target, params_t& params)
    {
        if (target.empty()) return handler;

        auto n = target.find('/');
        auto current_segment = target.substr(0, n);

        if (!parameter.empty())
        {
            params[parameter] = current_segment;
            current_segment = parameter;
        }

        if (auto it = next_fixed_node.find(std::string(current_segment)); it == next_fixed_node.end())
        {
            return Node::not_found_handler;
        }
        else
        {
            std::string_view rest_segment = (n == std::string::npos) ? "" : target.substr(n + 1);
            return it->second.find(rest_segment, params);
        }
    }
}

















//
//#include "../metrics/metrics_store.h"
//
//#include <fstream>
//#include <iostream>
//#include <iomanip>
//
//namespace nimlib::Server::Handlers
//{
//    HttpRouter::HttpRouter() = default;
//
//    HttpRouter::~HttpRouter() = default;
//
//    Response HttpRouter::route(Request& http_request)
//    {
//        if (http_request.target == "/metrics")
//        {
//            auto& metrics_store = nimlib::Server::Metrics::MetricsStore<long>::get_instance();
//            auto report = metrics_store.generate_stats_report();
//
//            Response http_response{};
//            http_response.status = 200;
//            http_response.reason = "OK";
//            http_response.headers["content-type"].push_back("text/plain");
//            http_response.body = report;
//            return http_response;
//        }
//        else if (http_request.target == "/files/img.jpg")
//        {
//            std::string contents;
//            std::ifstream in("./img.jpg", std::ios::in | std::ios::binary);
//            if (in)
//            {
//                in.seekg(0, std::ios::end);
//                contents.resize(in.tellg());
//                in.seekg(0, std::ios::beg);
//                in.read(&contents[0], contents.size());
//                in.close();
//            }
//
//            Response http_response{};
//            http_response.status = 200;
//            http_response.reason = "OK";
//            http_response.headers["content-type"].push_back("image/jpeg");
//            http_response.body = contents;
//            return http_response;
//        }
//        else if (http_request.target == "/login" && http_request.method == "POST")
//        {
//            std::cout << http_request.body << std::endl;
//            Response http_response{};
//            http_response.status = 404;
//            http_response.reason = "Not found";
//            http_response.headers["content-type"].push_back("text/html; charset=UTF-8");
//            http_response.body = "not found";
//            return http_response;
//        }
//        else
//        {
//            Response http_response{};
//            http_response.status = 404;
//            http_response.reason = "Not found";
//            http_response.headers["content-type"].push_back("text/html; charset=UTF-8");
//            http_response.body = "not found";
//            return http_response;
//        }
//    }
//}
