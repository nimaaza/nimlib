#pragma once

#include <sstream>
#include <memory>

#include <botan/tls_server.h>

#include "../common/types.h"

using nimlib::Server::Types::StreamsProvider;
using nimlib::Server::Types::Handler;
using nimlib::Server::Types::Connection;

namespace nimlib::Server::Handlers
{
	class TlsLayer : public Handler, public StreamsProvider
	{
	public:
		TlsLayer(
			Connection& connection,
			StreamsProvider& connection_encrypted_streams,
			std::shared_ptr<Handler> next = nullptr
		);
		~TlsLayer();

		void notify(Connection& connection, StreamsProvider& streams) override;
		void notify(Handler& handler, Connection& connection, StreamsProvider& streams) override;
		bool wants_more_bytes() override;
		bool wants_to_write() override;
		bool wants_to_live() override;

		std::stringstream& source() override;
		std::stringstream& sink() override;

	private:
		bool tls_continue{ true };
		StreamsProvider& connection_encrypted_streams;
		std::stringstream decrypted_input{};
		std::stringstream decrypted_output{};
		std::unique_ptr<Botan::TLS::Server> tls_server;
		std::shared_ptr<Handler> next;
	};
};
