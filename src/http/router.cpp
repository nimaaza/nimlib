#include "router.h"

#include <filesystem>
#include <fstream>

namespace nimlib::Server::Handlers::Http
{
    bool Router::get(std::string target, route_handler handler) { return add("GET", target, handler); }

    bool Router::post(std::string target, route_handler handler) { return add("POST", target, handler); }

    bool Router::serve_static(std::string target, std::string file)
    {
        auto static_handlder = [this](const Request& request, Response& response, params_t&) -> void
            {
                auto file = this->target_to_file[request.target];
                auto content_type = this->target_to_mime_type[request.target];

                std::string contents;
                std::ifstream in(file, std::ios::in | std::ios::binary);
                if (in)
                {
                    in.seekg(0, std::ios::end);
                    contents.resize(in.tellg());
                    in.seekg(0, std::ios::beg);
                    in.read(&contents[0], contents.size());
                    in.close();
                }

                response.status = 200;
                response.reason = "OK";
                response.headers["content-type"].push_back(content_type);
                response.body = contents;
            };

        if (valid_static_file(file) && add("GET", target, static_handlder))
        {
            target_to_file[target] = file;
            target_to_mime_type[target] = get_content_type(file);

            return true;
        }
        else
        {
            return false;
        }
    }

    void Router::fallback(route_handler handler) { fallback_handler = handler; }

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
            else if (fallback_handler)
            {
                fallback_handler(request, response, params);
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

    std::string Router::get_content_type(std::string file)
    {
        auto ext = std::filesystem::path(file).extension().string();

        if (auto it = ext_to_mime_type.find(ext); it != ext_to_mime_type.end())
        {
            return it->second;
        }
        else
        {
            return {};
        }
    }

    bool Router::valid_static_file(std::string file)
    {
        auto path = std::filesystem::path(file);
        return path.is_absolute()
            && std::filesystem::exists(path)
            && std::filesystem::is_regular_file(path)
            && !get_content_type(file).empty();
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
            return {};
        }
    }
}
