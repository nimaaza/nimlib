#include "tls_layer.h"
#include "botan/tls_server.h"

#include "iostream"

namespace nimlib::Server::Protocols
{
	TlsLayer::TlsLayer(
		ConnectionInterface& connection,
		StreamsProviderInterface& connection_streams,
		std::shared_ptr<ProtocolInterface> next
	) : connection_encrypted_streams{ connection_streams }
	{
		next = std::make_shared<Protocol>(connection, *this, *this);
		tls_server = nimlib::Server::Protocols::BotanSpec::get_tls_server(
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
			connection.notify(*this);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			connection.notify(*this);
		}
	}

	void TlsLayer::notify(ProtocolInterface& protocol, StreamsProviderInterface& streams) {}

	bool TlsLayer::wants_more_bytes() { return true; }

	bool TlsLayer::wants_to_write() { return true; }

	bool TlsLayer::wants_to_live() { return true; }

	std::stringstream& TlsLayer::get_input_stream() { return decrypted_input; }

	std::stringstream& TlsLayer::get_output_stream() { return decrypted_output; }
}
