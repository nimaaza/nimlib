#include "tls_layer.h"
#include "botan/tls_server.h"

#include "iostream"

namespace nimlib::Server::Protocols
{
	TlsLayer::TlsLayer(
		ConnectionInterface& connection,
		StreamsProviderInterface& streams,
		std::shared_ptr<ProtocolInterface> next
	) : in{ streams.get_input_stream() }, out{ streams.get_output_stream() }
	{
		next = std::make_shared<Protocol>(connection, streams);
		tls_server = nimlib::Server::Protocols::BotanSpec::get_tls_server(streams, *this, next);
	}

	TlsLayer::~TlsLayer() = default;

	void TlsLayer::notify(ConnectionInterface& connection)
	{
		try
		{
			std::string in_string{ in.str() };
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

	bool TlsLayer::wants_more_bytes() { return true; }

	bool TlsLayer::wants_to_write() { return true; }

	bool TlsLayer::wants_to_live() { return true; }

	std::stringstream& TlsLayer::get_input_stream() { return internal_in; }

	std::stringstream& TlsLayer::get_output_stream() { return internal_out; }
}
