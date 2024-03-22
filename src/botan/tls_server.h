#pragma once

#include <memory>

#include <botan/tls_server.h>

namespace nimlib::Server::Protocols::BotanSpec
{
	std::unique_ptr<Botan::TLS::Server> get_tls_server(const std::stringstream& in, std::stringstream& out);
};
