#pragma once

#include <memory>
#include <botan/tls_server.h>

#include "../../common/types.h"

using nimlib::Server::Types::Connection;
using nimlib::Server::Types::Handler;
using nimlib::Server::Types::StreamsProvider;

namespace nimlib::Server::Handlers::BotanSpec
{
    std::unique_ptr<Botan::TLS::Server> get_tls_server(
        Connection& connection,
        Handler& tls_layer,
        std::shared_ptr<Handler> next,
        StreamsProvider& encrypted_streams,
        StreamsProvider& decrypted_streams
    );
};
