#include "http.h"

#include <sstream>

namespace nimlib::Server::Protocols
{
	Http::Http(ConnectionInterface& connection) : connection{ connection } {}

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
		std::stringstream& output_to_tls{ streams.get_output_stream() };
		output_to_tls << "done_all";
		protocol.notify(*this, connection, streams);
	}

	bool Http::wants_more_bytes() { return false; }

	bool Http::wants_to_write() { return true; }

	bool Http::wants_to_live() { return false; }
}
