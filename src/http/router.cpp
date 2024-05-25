#include "router.h"

#include "../utils/helpers.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <iomanip>

namespace nimlib::Server::Handlers::Http
{
    // TODO: target should not end with /
    bool Router::get(std::string target, route_handler handler) { return add("GET", target, handler); }

    bool Router::post(std::string target, route_handler handler) { return add("POST", target, handler); }

    bool Router::serve_static(std::string target, std::string file)
    {
        auto static_handler = [this](const Request& request, Response& response, params_t&) -> std::optional<HandlerState>
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

                return HandlerState::FINISHED_NO_WAIT;
            };

        if (valid_static_file(file) && add("GET", target, static_handler))
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

    bool Router::serve_static_big(std::string target, std::string file)
    {
        auto a = [this](const Request& request, Response& response, params_t& params)-> std::optional<HandlerState>
            {
                return this->static_file_handler_big(request, response, params);
            };


        if (valid_static_file(file) && add("GET", target, a))
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

    void Router::sub_route(std::string target_prefix, Router sub_router)
    {
        const static std::string root_node_name = "";

        for (const auto& [method, node] : sub_router.handlers)
        {
            if (auto root_node_it = node.next.find(""); root_node_it != node.next.end())
            {
                if (auto it = handlers.find(method); it != handlers.end())
                {
                    it->second.add(target_prefix, root_node_it->second);
                }
                else
                {
                    Node new_node;
                    new_node.add(target_prefix, root_node_it->second);
                    handlers[method] = std::move(new_node);
                }
            }
        }
    }

    void Router::fallback(route_handler handler) { fallback_handler = handler; }

    std::optional<HandlerState> Router::route(const Request& request, Response& response)
    {
        if (auto it = handlers.find(request.method); it != handlers.end())
        {
            params_t params;
            auto handler = it->second.find(request.target, params);
            if (handler && handler.value())
            {
                return handler.value()(request, response, params);
            }
            else if (fallback_handler)
            {
                return fallback_handler(request, response, params);
            }
            else
            {
                return {};
            }
        }
        else
        {
            return {};
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

    std::optional<HandlerState> Router::static_file_handler_big(const Request& request, Response& response, params_t& params)
    {
        auto file = target_to_file[request.target];
        std::ifstream file_stream{ file, std::ios::in | std::ios::binary };

        std::stringstream body;
        std::string buffer;
        long chunk_size = 1024 * 300;
        long file_size = std::filesystem::file_size(std::filesystem::path(file));
        long expected_chunk_count = file_size / chunk_size + (file_size % chunk_size == 0 ? 0 : 1);
        int chunk_number = 0;
        bool has_chunk;

        if (auto it = last_served_chunk.find(file); it != last_served_chunk.end())
        {
            chunk_number = it->second;
        }

        while (true)
        {
            has_chunk = read_chunk(file_stream, buffer, file_size, chunk_size, chunk_number);
            if (has_chunk)
            {
                body << std::hex << buffer.size() << "\r\n" << buffer << "\r\n";
                chunk_number++;
            }

            if (!has_chunk || (chunk_number % 2 == 0))
            {
                break;
            }
        }

        response.status = 200;
        response.reason = "OK";
        response.headers["content-type"].push_back(target_to_mime_type[request.target]);
        response.headers["transfer-encoding"].push_back("chunked");

        if (chunk_number == expected_chunk_count)
        {
            body << "0\r\n\r\n";
            response.body = body.str();
            last_served_chunk.erase(file);
            return HandlerState::FINISHED_NO_WAIT;
        }
        else
        {
            response.body = body.str();
            last_served_chunk[file] = chunk_number;
            return HandlerState::RECALL;
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

    void Router::Node::add(std::string target, Node node)
    {
        size_t pos = target.find('/');
        std::string target_segment = target.substr(0, pos);
        std::string target_rest = (pos == std::string::npos) ? "" : target.substr(pos + 1);

        if (target_segment.starts_with('<') && target_segment.ends_with('>'))
        {
            parameter = target_segment.substr(1, target_segment.size() - 2);
            target_segment = parameter;
        }

        if (target_rest.empty())
        {
            next[target_segment] = std::move(node);
        }
        else
        {
            if (auto it = next.find(target_segment); it != next.end())
            {
                it->second.add(target_rest, std::move(node));
            }
            else
            {
                Node new_node;
                new_node.add(target_rest, std::move(node));
                next[target_segment] = std::move(new_node);
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
