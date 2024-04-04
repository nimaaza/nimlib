#pragma once

#include <sstream>
#include <memory>

#include <botan/tls_server.h>

#include "http.h"
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
			StreamsProviderInterface& connection_encrypted_streams,
			std::shared_ptr<ProtocolInterface> next = nullptr
		);
		~TlsLayer();

		void notify(
			ConnectionInterface& connection,
			StreamsProviderInterface& streams
		) override;
		void notify(
			ProtocolInterface& protocol,
			ConnectionInterface& connection,
			StreamsProviderInterface& streams
		) override;
		bool wants_more_bytes() override;
		bool wants_to_write() override;
		bool wants_to_live() override;

		std::stringstream& get_input_stream() override;
		std::stringstream& get_output_stream() override;

	private:
		StreamsProviderInterface& connection_encrypted_streams;
		std::stringstream decrypted_input{};
		std::stringstream decrypted_output{};
		std::unique_ptr<Botan::TLS::Server> tls_server;
		std::shared_ptr<Http> next;
	};
};
