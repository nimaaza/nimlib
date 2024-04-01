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

		void notify(ConnectionInterface& connection) override;
		bool wants_more_bytes() override;
		bool wants_to_write() override;
		bool wants_to_live() override;
	};
};
