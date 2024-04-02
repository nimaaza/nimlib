#pragma once

#include <memory>

#include <botan/tls_server.h>

#include "../types.h"

using nimlib::Server::Types::ProtocolInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols::BotanSpec
{
    std::unique_ptr<Botan::TLS::Server> get_tls_server(
        StreamsProviderInterface& source_streams,
        StreamsProviderInterface& internal_streams,
        std::shared_ptr<ProtocolInterface> next);
};
