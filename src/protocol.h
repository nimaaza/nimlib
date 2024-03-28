#pragma once

#include <sstream>

#include "types.h"

using nimlib::Server::Types::ConnectionInterface;

namespace nimlib::Server::Protocols
{
	using nimlib::Server::Types::ProtocolInterface;
	using ParseResult = nimlib::Server::Constants::ParseResult;

	class Protocol : public ProtocolInterface
	{
	public:
		explicit Protocol(std::stringstream& in, std::stringstream& out);
		~Protocol();

		void parse(ConnectionInterface& connection) override;
	};
};
