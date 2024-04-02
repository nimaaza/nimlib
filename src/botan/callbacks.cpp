#include "callbacks.h"

namespace nimlib::Server::Protocols::BotanSpec
{
	Callbacks::Callbacks(
		StreamsProviderInterface& source_streams,
		StreamsProviderInterface& internal_streams,
		std::shared_ptr<ProtocolInterface> next
	) :
		source_in{ source_streams.get_input_stream() },
		source_out{ source_streams.get_output_stream() },
		internal_in{ internal_streams.get_input_stream() },
		internal_out{ internal_streams.get_output_stream() },
		next{ next }
	{}

	void Callbacks::tls_emit_data(std::span<const uint8_t> data)
	{
		for (auto c : data)
		{
			source_out << c;
		}
	}

	void Callbacks::tls_record_received(uint64_t seq_no, std::span<const uint8_t> data)
	{
		for (auto c : data)
		{
			internal_in << c;
		}
	}

	void Callbacks::tls_alert(Botan::TLS::Alert alert) {}

	void Callbacks::tls_session_established(const Botan::TLS::Session_Summary& session) {}
}
