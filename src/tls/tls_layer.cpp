#include "tls_layer.h"
#include "botan/tls_server.h"

namespace nimlib::Server::Protocols
{
	TlsLayer::TlsLayer(
		ConnectionInterface& connection,
		StreamsProviderInterface& connection_encrypted_streams,
		std::shared_ptr<ProtocolInterface> next
	) :
		connection_encrypted_streams{ connection_encrypted_streams },
		next{ next }
	{
		tls_server = nimlib::Server::Protocols::BotanSpec::get_tls_server(
			connection,
			*this,
			connection_encrypted_streams,
			*this,
			next
		);
	}

	TlsLayer::~TlsLayer() = default;

	void TlsLayer::notify(ConnectionInterface& connection, StreamsProviderInterface& streams)
	{
		try
		{
			std::stringstream& encrypted_input{ connection_encrypted_streams.get_input_stream() };
			std::string in_string{ encrypted_input.str() };
			auto in_string_ptr = reinterpret_cast<uint8_t*>(in_string.data());
			auto bytes_needed = tls_server->received_data(in_string_ptr, in_string.size());
			tls_continue = bytes_needed > 0;
			connection.notify(*this);
		}
		catch (const std::exception& e)
		{
			// TODO: connection needs to be set in error mode in this case and closed.
			connection.notify(*this);
		}
	}

	void TlsLayer::notify(
		ProtocolInterface& protocol,
		ConnectionInterface& connection,
		StreamsProviderInterface& streams
	)
	{
		if (next->wants_to_write())
		{
			tls_server->send(streams.get_output_stream().str());
		}
		tls_server->close();
	}

	bool TlsLayer::wants_more_bytes() { return tls_continue || next->wants_more_bytes(); }

	bool TlsLayer::wants_to_write() { return tls_continue || next->wants_to_write(); }

	bool TlsLayer::wants_to_live() { return tls_continue || next->wants_to_live(); }

	std::stringstream& TlsLayer::get_input_stream() { return decrypted_input; }

	std::stringstream& TlsLayer::get_output_stream() { return decrypted_output; }
}
