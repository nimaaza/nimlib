#include "router.h"

namespace nimlib::Server::Handlers::Http
{
    bool Router::get(std::string target, route_handler handler) { return add("GET", target, handler); }

    bool Router::post(std::string target, route_handler handler) { return add("POST", target, handler); }

    void Router::fallback(route_handler fallback_handler) { add_fallback("POST", fallback_handler); }

    bool Router::route(const Request& request, Response& response)
    {
        if (auto it = handlers.find(request.method); it != handlers.end())
        {
            params_t params;
            auto handler = it->second.find(request.target, params);
            if (handler && handler.value())
            {
                handler.value()(request, response, params);
                return true;
            }
            else
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    bool Router::add(std::string method, std::string target, route_handler handler)
    {
        // TODO: also check if method is valid?
        if (target.empty()) return false;

        if (auto it = handlers.find(method); it != handlers.end())
        {
            it->second.add(target, handler);
        }
        else
        {
            Node node;
            node.add(target, handler);
            handlers[method] = std::move(node);
        }

        return true;
    }

    void Router::add_fallback(std::string method, route_handler handler)
    {
        if (auto it = handlers.find(method); it != handlers.end())
        {
            it->second.fallback_handler = handler;
        }
        else
        {
            Node node;
            node.fallback_handler = handler;
            handlers[method] = std::move(node);
        }
    }

    Router::Node::Node(std::string target, route_handler h) { add(target, h); }

    void Router::Node::add(std::string target, route_handler h)
    {
        if (target.empty())
        {
            handler = h;
        }
        else
        {
            size_t pos = target.find('/');
            std::string target_segment = target.substr(0, pos);
            std::string target_rest = (pos == std::string::npos) ? "" : target.substr(pos + 1);

            if (target_segment.starts_with('<') && target_segment.ends_with('>'))
            {
                parameter = target_segment.substr(1, target_segment.size() - 2);
                target_segment = parameter;
            }

            if (auto it = next.find(target_segment); it != next.end())
            {
                it->second.add(target_rest, h);
            }
            else
            {
                next[target_segment] = std::move(Node(target_rest, h));
            }
        }
    }

    std::optional<route_handler> Router::Node::find(std::string_view target, params_t& params)
    {
        if (target.empty()) return handler;

        size_t pos = target.find('/');
        std::string_view target_segment = target.substr(0, pos);

        if (!parameter.empty())
        {
            params[parameter] = target_segment;
            target_segment = parameter;
        }

        if (auto it = next.find(std::string(target_segment)); it != next.end())
        {
            std::string_view rest_segment = (pos == std::string::npos) ? "" : target.substr(pos + 1);
            return it->second.find(rest_segment, params);
        }
        else
        {
            return Node::fallback_handler;
        }
    }
}

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
