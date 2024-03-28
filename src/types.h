#pragma once

#include <span>
#include <sys/socket.h>

#include "common.h"

namespace nimlib::Server::Types
{
	using nimlib::Server::Constants::ConnectionState;
	using nimlib::Server::Constants::ParseResult;

	class ConnectionInterface;

	struct PollingServerInterface
	{
		virtual ~PollingServerInterface() = default;

		virtual void run() = 0;
	};

	struct TcpSocketInterface
	{
		TcpSocketInterface(const std::string& port, int tcp_socket_descriptor)
			: port{ port }, tcp_socket_descriptor{ tcp_socket_descriptor }
		{}
		virtual ~TcpSocketInterface() = default;

		virtual int tcp_bind() = 0;
		virtual int tcp_listen() = 0;
		virtual std::unique_ptr<TcpSocketInterface> tcp_accept() = 0;
		virtual void tcp_get_host_name(const sockaddr& socket_address, std::string& host_name) = 0;
		virtual int tcp_read(std::span<uint8_t> buffer, int flags) = 0;
		virtual int tcp_send(std::span<uint8_t> buffer) = 0;
		virtual int tcp_send(std::string_view buffer) = 0;
		virtual void tcp_close() = 0;  // TODO: may never be used
		virtual const int get_tcp_socket_descriptor() const = 0;
		virtual const std::string& get_port() const = 0;

	protected:
		const std::string& port;
		int tcp_socket_descriptor;
	};

	struct ProtocolInterface
	{
		ProtocolInterface(
			std::stringstream& in,
			std::stringstream& out,
			std::shared_ptr<ProtocolInterface> next = nullptr
		) : in{ in }, out{ out }, next{ next }
		{};
		virtual ~ProtocolInterface() = default;

		virtual void parse(ConnectionInterface& connection) = 0;

	protected:
		std::stringstream& in;
		std::stringstream& out;
		std::shared_ptr<ProtocolInterface> next;
	};

	struct ConnectionInterface
	{
	public:
		virtual ~ConnectionInterface() = default;

		virtual ConnectionState read() = 0;
		virtual ConnectionState write() = 0;
		virtual void halt() = 0;
		virtual void set_parse_state(ParseResult pr) = 0;
		// virtual ConnectionInterface& operator<<(uint8_t c) = 0;
		// virtual ConnectionInterface& operator<<(std::string& s) = 0;
		virtual void set_protocol(std::shared_ptr<ProtocolInterface>) = 0;
		virtual std::pair<ConnectionState, long> get_state() const = 0;
		virtual const int get_id() const = 0;
	};
};

using connection_ptr = std::shared_ptr<nimlib::Server::Types::ConnectionInterface>;
using socket_ptr = std::unique_ptr<nimlib::Server::Types::TcpSocketInterface>;
using connection_id = int;
