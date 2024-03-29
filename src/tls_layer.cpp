#include "tls_layer.h"
#include "botan/tls_server.h"

#include "iostream"

namespace nimlib::Server::Protocols
{
	TlsLayer::TlsLayer(ConnectionInterface& connection, std::shared_ptr<ProtocolInterface> next)
		: in{ connection.get_input_stream() }, out{ connection.get_output_stream() }
	{
		tls_server = nimlib::Server::Protocols::BotanSpec::get_tls_server(in, out, next);
	}

	TlsLayer::~TlsLayer() = default;

	void TlsLayer::parse(ConnectionInterface& connection)
	{
		try
		{
			std::string in_string{ in.str() };
			auto in_string_ptr = reinterpret_cast<uint8_t*>(in_string.data());
			auto bytes_needed = tls_server->received_data(in_string_ptr, in_string.size());
			connection.set_parse_state(ParseResult::WRITE_AND_WAIT);
		}
		catch (const std::exception& e)
		{
			std::cout << e.what() << std::endl;
			connection.set_parse_state(ParseResult::WRITE_AND_DIE);
		}
	}
}