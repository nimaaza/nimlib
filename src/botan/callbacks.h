#pragma once

#include <sstream>

#include <botan/tls_callbacks.h>

namespace nimlib::Server::Protocols::BotanSpec
{
	class Callbacks : public Botan::TLS::Callbacks
	{
	public:
		Callbacks(const std::stringstream& in, std::stringstream& out);
		~Callbacks() = default;

		void tls_emit_data(std::span<const uint8_t> data) override;
		void tls_record_received(uint64_t seq_no, std::span<const uint8_t> data) override;
		void tls_alert(Botan::TLS::Alert alert) override;
		void tls_session_established(const Botan::TLS::Session_Summary& session) override;

	private:
		std::stringstream& out;
		const std::stringstream& in;
	};
}
