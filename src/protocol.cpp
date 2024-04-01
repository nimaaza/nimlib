#include "protocol.h"

#include <sstream>

namespace nimlib::Server::Protocols
{
	Protocol::Protocol() = default;

	Protocol::~Protocol() = default;

	void Protocol::notify(ConnectionInterface& connection)
	{
		std::stringstream& out{ connection.get_output_stream() };
		out << "done";
		connection.notify();
	}

	bool Protocol::wants_more_bytes() { return false; }

	bool Protocol::wants_to_write() { return true; }

	bool Protocol::wants_to_live() { return false; }
}
