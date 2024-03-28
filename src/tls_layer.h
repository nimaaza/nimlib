#pragma once

#include <sstream>
#include <memory>

#include <botan/tls_server.h>

#include "protocol.h"

namespace nimlib::Server::Protocols
{
	class TlsLayer : public ProtocolInterface
	{
	public:
		TlsLayer(std::stringstream& in, std::stringstream& out, std::shared_ptr<ProtocolInterface> next);
		~TlsLayer();

		void parse(ConnectionInterface& connection) override;

	private:
		std::unique_ptr<Botan::TLS::Server> tls_server;
	};
};
