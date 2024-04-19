#pragma once

#include "http_parser.h"
#include "../common/types.h"

using nimlib::Server::Types::ConnectionInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols
{
    using nimlib::Server::Types::ProtocolInterface;
    using ParseResult = nimlib::Server::Constants::ParseResult;
    class HttpRequest;

    class Http : public ProtocolInterface
    {
    public:
        Http(ConnectionInterface& connection);
        ~Http();

        void notify(ConnectionInterface& connection, StreamsProviderInterface& streams) override;
        void notify(ProtocolInterface& protocol, ConnectionInterface& connection, StreamsProviderInterface& streams) override;
        bool wants_more_bytes() override;
        bool wants_to_write() override;
        bool wants_to_live() override;

    private:
        ConnectionInterface& connection;
        std::optional<HttpRequest> http_request{ std::nullopt };
    };
};
