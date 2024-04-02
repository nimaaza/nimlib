#pragma once

#include <sstream>

#include <botan/tls_callbacks.h>

#include "../types.h"

using nimlib::Server::Types::ProtocolInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols::BotanSpec
{
	class Callbacks : public Botan::TLS::Callbacks
	{
	public:
		Callbacks(
			StreamsProviderInterface& source_streams,
			StreamsProviderInterface& internal_streams,
			std::shared_ptr<ProtocolInterface> next
		);
		~Callbacks() = default;

		void tls_emit_data(std::span<const uint8_t> data) override;
		void tls_record_received(uint64_t seq_no, std::span<const uint8_t> data) override;
		void tls_alert(Botan::TLS::Alert alert) override;
		void tls_session_established(const Botan::TLS::Session_Summary& session) override;

	private:
		std::stringstream& internal_out;
		std::stringstream& internal_in;
		std::stringstream& source_out;
		std::stringstream& source_in;
		std::shared_ptr<ProtocolInterface> next;
	};
}
