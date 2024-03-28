#include <array>

#include "protocol.h"

namespace nimlib::Server::Protocols
{
	Protocol::Protocol(std::stringstream& in, std::stringstream& out)
		: ProtocolInterface{ in, out }
	{}

	Protocol::~Protocol() {}

	void Protocol::parse(ConnectionInterface& connection)
	{
		out << "done";
		connection.set_parse_state(ParseResult::WRITE_AND_DIE);
	}
}
