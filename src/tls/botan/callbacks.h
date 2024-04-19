#pragma once

#include <sstream>

#include <botan/tls_callbacks.h>

#include "../../common/types.h"

using nimlib::Server::Types::ProtocolInterface;
using nimlib::Server::Types::ConnectionInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols::BotanSpec
{
	class Callbacks : public Botan::TLS::Callbacks
	{
	public:
		Callbacks(
			ConnectionInterface& connection,
			ProtocolInterface& tls_layer,
			StreamsProviderInterface& encrypted_streams,
			StreamsProviderInterface& decrypted_streams,
			std::shared_ptr<ProtocolInterface> next
		);
		~Callbacks() = default;

		void tls_emit_data(std::span<const uint8_t> data) override;
		void tls_record_received(uint64_t seq_no, std::span<const uint8_t> data) override;
		void tls_alert(Botan::TLS::Alert alert) override;
		void tls_session_established(const Botan::TLS::Session_Summary& session) override;

	private:
		ConnectionInterface& connection;
		ProtocolInterface& tls_layer;
		StreamsProviderInterface& encrypted_streams;
		StreamsProviderInterface& decrypted_streams;
		std::shared_ptr<ProtocolInterface> next;
	};
}
