#pragma once

#include "parser.h"

#include <unordered_map>

namespace nimlib::Server::Handlers::Http
{
    using params_t = std::unordered_map<std::string, std::string>;
    using route_handler = std::function<void(const Request&, Response&, params_t&)>;

    class Router
    {
        class Node;

    public:
        Router() = default;
        ~Router() = default;

        bool get(std::string target, route_handler handler);
        bool post(std::string target, route_handler handler);
        void fallback(route_handler fallback_handler);

        bool route(const Request&, Response&);

    private:
        bool add(std::string method, std::string target, route_handler handler);
        void add_fallback(std::string method, route_handler handler);

    private:
        std::unordered_map<std::string, Node> handlers{};

        class Node
        {
        public:
            Node(std::string target, route_handler h);
            Node() = default;
            ~Node() = default;

            Node(const Node&) = delete;
            Node& operator=(const Node&) = delete;
            Node(Node&&) noexcept = default;
            Node& operator=(Node&&) noexcept = default;

            void add(std::string target, route_handler h);
            std::optional<route_handler> find(std::string_view target, params_t& params);

        public:
            inline static route_handler fallback_handler = {};

        private:
            std::unordered_map<std::string, Node> next{};
            std::optional<route_handler> handler{};
            std::string parameter{};
        };
    };
};
