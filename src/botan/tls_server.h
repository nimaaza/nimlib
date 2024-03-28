#pragma once

#include <memory>

#include <botan/tls_server.h>

#include "../types.h"

using nimlib::Server::Types::ProtocolInterface;

namespace nimlib::Server::Protocols::BotanSpec
{
	std::unique_ptr<Botan::TLS::Server> get_tls_server(
		const std::stringstream& in,
		std::stringstream& out,
		std::shared_ptr<ProtocolInterface> next);
};
