#include "callbacks.h"

#include "iostream"

namespace nimlib::Server::Protocols::BotanSpec
{
	Callbacks::Callbacks(
		const std::stringstream& in,
		std::stringstream& out,
		std::shared_ptr<ProtocolInterface> next)
		: in{ in }, out{ out }, next{ next }
	{}

	void Callbacks::tls_emit_data(std::span<const uint8_t> data)
	{
		for (auto c : data)
		{
			out << c;
		}
	}

	void Callbacks::tls_record_received(uint64_t seq_no, std::span<const uint8_t> data)
	{
		for (auto c : data) std::cout << c;
	}

	void Callbacks::tls_alert(Botan::TLS::Alert alert) {}

	void Callbacks::tls_session_established(const Botan::TLS::Session_Summary& session) {}
}
