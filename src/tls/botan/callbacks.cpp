#include "callbacks.h"

namespace nimlib::Server::Protocols::BotanSpec
{
	Callbacks::Callbacks(
		ConnectionInterface& connection,
		ProtocolInterface& tls_layer,
		StreamsProviderInterface& encrypted_streams,
		StreamsProviderInterface& decrypted_streams,
		std::shared_ptr<ProtocolInterface> next
	) :
		connection{ connection },
		tls_layer{ tls_layer },
		encrypted_streams{ encrypted_streams },
		decrypted_streams{ decrypted_streams },
		next{ next }
	{}

	void Callbacks::tls_emit_data(std::span<const uint8_t> data)
	{
		std::stringstream& encrypted_output{ encrypted_streams.get_output_stream() };

		for (auto c : data)
		{
			encrypted_output << c;
		}

		connection.notify(tls_layer);
	}

	void Callbacks::tls_record_received(uint64_t seq_no, std::span<const uint8_t> data)
	{
		std::stringstream& decrypted_input{ decrypted_streams.get_input_stream() };

		for (auto c : data)
		{
			decrypted_input << c;
		}

		next->notify(tls_layer, connection, decrypted_streams);
	}

	void Callbacks::tls_alert(Botan::TLS::Alert alert) {}

	void Callbacks::tls_session_established(const Botan::TLS::Session_Summary& session) {}
}
