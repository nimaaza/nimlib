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
        Router();
        ~Router() = default;

        // These methods are used for setting up routes.
        void get(std::string target, route_handler handler);
        void get(route_handler not_found_handler);
        void post(std::string target, route_handler handler);
        void post(route_handler not_found_handler);

        // These methods are used for invoking the handlers.
        void get(std::string target, const Request& request, Response& response);
        void post(std::string target, const Request& request, Response& response);

    private:
        void add(std::string method, std::string target, route_handler handler);
        void add_not_found(std::string method, route_handler handler);
        void handle(std::string method, std::string target, const Request& request, Response& response);

    private:
        std::unordered_map<std::string, Node> http_method_handlers{};

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
            static route_handler not_found_handler;

        private:
            std::unordered_map<std::string, Node> next_fixed_node{};
            std::optional<route_handler> handler{};
            std::string parameter{};
        };
    };
};
