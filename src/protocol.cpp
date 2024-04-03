#include "protocol.h"

#include <sstream>
#include <iostream>

namespace nimlib::Server::Protocols
{
	Protocol::Protocol(
		ConnectionInterface& connection,
		ProtocolInterface& tls_layer,
		StreamsProviderInterface& tls_decrypted_streams
	)
		: connection{ connection }, tls_layer{ tls_layer }, decrypted_streams{ tls_decrypted_streams }
	{}

	Protocol::~Protocol() = default;

	void Protocol::notify(ConnectionInterface& connection, StreamsProviderInterface& streams)
	{
		std::stringstream& out{ streams.get_output_stream() };
		out << "done";
		connection.notify(*this);
	}

	void Protocol::notify(ProtocolInterface& protocol, StreamsProviderInterface& streams)
	{
		std::stringstream& input_from_tls{ streams.get_input_stream() };
		std::cout << input_from_tls.str();
	}

	bool Protocol::wants_more_bytes() { return false; }

	bool Protocol::wants_to_write() { return true; }

	bool Protocol::wants_to_live() { return false; }
}
