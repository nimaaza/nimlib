#include "protocol.h"

#include <sstream>

namespace nimlib::Server::Protocols
{
	Protocol::Protocol() = default;

	Protocol::~Protocol() = default;

	void Protocol::parse(ConnectionInterface& connection)
	{
		std::stringstream& out { connection.get_output_stream() };
		out << "done";
		connection.set_parse_state(ParseResult::WRITE_AND_DIE);
	}
}
