#pragma once

#include "parser.h"
#include "../utils/state_manager.h"

#include <unordered_map>

namespace nimlib::Server::Handlers::Http
{
    using nimlib::Server::Constants::HandlerState;
    using params_t = std::unordered_map<std::string, std::string>;
    using route_handler = std::function<std::optional<HandlerState>(const Request&, Response&, params_t&)>;

    class Router
    {
        class Node;

    public:
        Router() = default;
        ~Router() = default;

        Router(const Router&) = default;
        Router& operator=(const Router&) = default;
        Router(Router&&) noexcept = default;
        Router& operator=(Router&&) = default;

        bool get(std::string target, route_handler handler);
        bool post(std::string target, route_handler handler);
        bool serve_static(std::string target, std::string file);
        bool serve_static_big(std::string target, std::string file);
        void sub_route(std::string target_prefix, Router sub_router);
        void fallback(route_handler fallback_handler);
        std::optional<HandlerState> route(const Request&, Response&);

    private:
        bool add(std::string method, std::string target, route_handler handler);
        std::string get_content_type(std::string file);
        std::optional<HandlerState> static_file_handler_big(const Request&, Response&, params_t&);
        bool valid_static_file(std::string file);

    private:
        route_handler fallback_handler{};
        std::unordered_map<std::string, Node> handlers{};
        std::unordered_map<std::string, std::string> target_to_file{};
        std::unordered_map<std::string, std::string> target_to_mime_type{};
        std::unordered_map<std::string, int> last_served_chunk{};
        inline static const std::unordered_map<std::string, std::string> ext_to_mime_type
        {
            {".jpg", "image/jpeg"},
            {".jpeg", "image/jpeg"},
            {".mp4", "video/mp4"}
        };

    private:
        struct Node
        {
            Node(std::string target, route_handler h);
            Node() = default;
            ~Node() = default;

            Node(const Node&) = default;
            Node& operator=(const Node&) = default;
            Node(Node&&) noexcept = default;
            Node& operator=(Node&&) noexcept = default;

            void add(std::string target, route_handler h);
            void add(std::string target, Node node);
            std::optional<route_handler> find(std::string_view target, params_t& params);

            std::unordered_map<std::string, Node> next{};
            std::optional<route_handler> handler{};
            std::string parameter{};
        };
    };
};
