#include "protocol.h"

#include <sstream>

namespace nimlib::Server::Protocols
{
	Protocol::Protocol(ConnectionInterface& connection, StreamsProviderInterface& streams)
		: connection{ connection }, streams{ streams }
	{}

	Protocol::~Protocol() = default;

	void Protocol::notify(ConnectionInterface& connection)
	{
		std::stringstream& out{ streams.get_output_stream() };
		out << "done";
		connection.notify(*this);
	}

	bool Protocol::wants_more_bytes() { return false; }

	bool Protocol::wants_to_write() { return true; }

	bool Protocol::wants_to_live() { return false; }
}
