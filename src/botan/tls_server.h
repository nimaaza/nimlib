#pragma once

#include <memory>

#include <botan/tls_server.h>

#include "../common/types.h"

using nimlib::Server::Types::ConnectionInterface;
using nimlib::Server::Types::ProtocolInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols::BotanSpec
{
    std::unique_ptr<Botan::TLS::Server> get_tls_server(
        ConnectionInterface& connection,
        ProtocolInterface& tls_layer,
        StreamsProviderInterface& encrypted_streams,
        StreamsProviderInterface& decrypted_streams,
        std::shared_ptr<ProtocolInterface> next);
};
