#pragma once

#include "http_parser.h"
#include "http_router.h"
#include "../common/types.h"

using nimlib::Server::Types::Connection;
using nimlib::Server::Types::StreamsProvider;

namespace nimlib::Server::Handlers
{
    using nimlib::Server::Types::Handler;
    using ParseResult = nimlib::Server::Constants::ParseResult;
    class HttpRequest;

    class Http : public Handler
    {
    public:
        Http(Connection& connection);
        ~Http();

        void notify(Connection& connection, StreamsProvider& streams) override;
        void notify(Handler& handler, Connection& connection, StreamsProvider& streams) override;
        bool wants_more_bytes() override;
        bool wants_to_write() override;
        bool wants_to_live() override;

    private:
        Connection& connection;
        std::optional<HttpRequest> http_request{ std::nullopt };
    };
};
