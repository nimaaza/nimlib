#pragma once

#include "types.h"

using nimlib::Server::Types::ConnectionInterface;

namespace nimlib::Server::Protocols
{
	using nimlib::Server::Types::ProtocolInterface;
	using ParseResult = nimlib::Server::Constants::ParseResult;

	class Protocol : public ProtocolInterface
	{
	public:
		explicit Protocol();
		~Protocol();

		void parse(ConnectionInterface& connection) override;
	};
};
