#pragma once

#include "types.h"

using nimlib::Server::Types::ConnectionInterface;
using nimlib::Server::Types::StreamsProviderInterface;

namespace nimlib::Server::Protocols
{
	using nimlib::Server::Types::ProtocolInterface;
	using ParseResult = nimlib::Server::Constants::ParseResult;

	class Http : public ProtocolInterface
	{
	public:
		Http(
			ConnectionInterface& connection,
			ProtocolInterface& tls_layer,
			StreamsProviderInterface& tls_decrypted_streams
		);
		~Http();

		void notify(ConnectionInterface& connection, StreamsProviderInterface& streams) override;
		void notify(ProtocolInterface& protocol, StreamsProviderInterface& streams) override;
		bool wants_more_bytes() override;
		bool wants_to_write() override;
		bool wants_to_live() override;

	private:
		ConnectionInterface& connection;
		ProtocolInterface& tls_layer;
		StreamsProviderInterface& decrypted_streams;
	};
};
