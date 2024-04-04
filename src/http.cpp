#include "http.h"

#include <sstream>

namespace nimlib::Server::Protocols
{
	Http::Http(
		ConnectionInterface& connection,
		ProtocolInterface& tls_layer,
		StreamsProviderInterface& tls_decrypted_streams
	)
		: connection{ connection },
		tls_layer{ tls_layer },
		decrypted_streams{ tls_decrypted_streams }
	{}

	Http::~Http() = default;

	void Http::notify(ConnectionInterface& connection, StreamsProviderInterface& streams)
	{
		std::stringstream& out{ streams.get_output_stream() };
		out << "done";
		connection.notify(*this);
	}

	void Http::notify(
		ProtocolInterface& protocol,
		ConnectionInterface& connection,
		StreamsProviderInterface& streams
	)
	{
		std::stringstream& input_from_tls{ streams.get_input_stream() };
		std::stringstream& output_to_tls{ decrypted_streams.get_output_stream() };
		output_to_tls << "done_all";
		tls_layer.notify(*this, connection, decrypted_streams);
	}

	bool Http::wants_more_bytes() { return false; }

	bool Http::wants_to_write() { return true; }

	bool Http::wants_to_live() { return false; }
}
