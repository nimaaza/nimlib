#pragma once

#include "parser.h"
#include "router.h"
#include "../utils/state_manager.h"
#include "../common/types.h"

using nimlib::Server::Constants::HandlerState;
using nimlib::Server::Utils::StateManager;
using nimlib::Server::Types::Connection;
using nimlib::Server::Types::StreamsProvider;

namespace nimlib::Server::Handlers::Http
{
    using nimlib::Server::Types::Handler;
    using nimlib::Server::Constants::HandlerState;
    class Request;

    class HttpHandler : public Handler
    {
    public:
        HttpHandler();
        ~HttpHandler() = default;

        void notify(Connection& connection, StreamsProvider& streams) override;
        void notify(Handler& handler, Connection& connection, StreamsProvider& streams) override;
        bool wants_more_bytes() override;
        bool wants_to_write() override;
        bool wants_to_live() override;
        bool wants_to_be_calledback() override;

        HandlerState get_state() override;

    private:
        std::optional<Request> http_request{ std::nullopt };
        std::optional<Response> htt_response{ std::nullopt };
        Router router{};
    };
};
