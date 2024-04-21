#pragma once

#include <sstream>

#include <botan/tls_callbacks.h>

#include "../../common/types.h"

using nimlib::Server::Types::Handler;
using nimlib::Server::Types::Connection;
using nimlib::Server::Types::StreamsProvider;

namespace nimlib::Server::Handlers::BotanSpec
{
	class Callbacks : public Botan::TLS::Callbacks
	{
	public:
		Callbacks(
			Connection& connection,
			Handler& tls_layer,
			StreamsProvider& encrypted_streams,
			StreamsProvider& decrypted_streams,
			std::shared_ptr<Handler> next
		);
		~Callbacks() = default;

		void tls_emit_data(std::span<const uint8_t> data) override;
		void tls_record_received(uint64_t seq_no, std::span<const uint8_t> data) override;
		void tls_alert(Botan::TLS::Alert alert) override;
		void tls_session_established(const Botan::TLS::Session_Summary& session) override;

	private:
		Connection& connection;
		Handler& tls_layer;
		StreamsProvider& encrypted_streams;
		StreamsProvider& decrypted_streams;
		std::shared_ptr<Handler> next;
	};
}
