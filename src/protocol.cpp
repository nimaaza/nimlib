#include <array>

#include "protocol.h"

namespace nimlib::Server::Protocols
{
	Protocol::Protocol(std::stringstream& in, std::stringstream& out)
		: ProtocolInterface {in, out}
	{
	}

	Protocol::~Protocol()
	{
	}

	ParseResult Protocol::parse()
	{
		std::array<char, 4> s{ 'd', 'o', 'n', 'e' };
		for (auto c : s) out << c;
		return ParseResult::WRITE_AND_DIE;
	}
}

// =======================================================================
// class HttpRouter {};
// class HttpParser {};
// class HttpObject {};
// class HttpRequest {};
// class HttpResponse {};
// =======================================================================
