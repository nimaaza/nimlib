#pragma once

#include <sstream>
#include <memory>

#include <botan/tls_server.h>

#include "protocol.h"
#include "types.h"

using nimlib::Server::Types::ConnectionInterface;

namespace nimlib::Server::Protocols
{
	class TlsLayer : public ProtocolInterface
	{
	public:
		TlsLayer(ConnectionInterface& connection, std::shared_ptr<ProtocolInterface> next = nullptr);
		~TlsLayer();

		void parse(ConnectionInterface& connection) override;

	private:
		std::stringstream& in;
		std::stringstream& out;
		std::unique_ptr<Botan::TLS::Server> tls_server;
	};
};
