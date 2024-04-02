#pragma once

#include <sstream>
#include <memory>

#include <botan/tls_server.h>

#include "protocol.h"
#include "types.h"

using nimlib::Server::Types::StreamsProviderInterface;
using nimlib::Server::Types::ConnectionInterface;

namespace nimlib::Server::Protocols
{
	class TlsLayer : public ProtocolInterface, public StreamsProviderInterface
	{
	public:
		TlsLayer(
			ConnectionInterface& connection,
			StreamsProviderInterface& streams,
			std::shared_ptr<ProtocolInterface> next = nullptr
		);
		~TlsLayer();

		void notify(ConnectionInterface& connection) override;
		bool wants_more_bytes() override;
		bool wants_to_write() override;
		bool wants_to_live() override;

		std::stringstream& get_input_stream() override;
		std::stringstream& get_output_stream() override;

	private:
		std::stringstream internal_in{};
		std::stringstream internal_out{};
		std::stringstream& in;
		std::stringstream& out;
		std::unique_ptr<Botan::TLS::Server> tls_server;
	};
};
