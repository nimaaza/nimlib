#pragma once

#include <span>
#include <sys/socket.h>
#include <functional>
#include <vector>
#include <unordered_map>

#include "common.h"
#include "../utils/state_manager.h"

namespace nimlib::Server::Types
{
	using nimlib::Server::Constants::ServerDirective;
	using nimlib::Server::Constants::ConnectionState;
	using nimlib::Server::Constants::ParseResult;

	class Connection;

	struct Server
	{
		virtual ~Server() = default;

		virtual void run() = 0;
	};

	struct Socket
	{
		Socket(const std::string& port, int tcp_socket_descriptor)
			: port{ port }, tcp_socket_descriptor{ tcp_socket_descriptor }
		{}
		virtual ~Socket() = default;

		virtual int tcp_bind() = 0;
		virtual int tcp_listen() = 0;
		virtual std::unique_ptr<Socket> tcp_accept() = 0;
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

	struct StreamsProvider
	{
		virtual ~StreamsProvider() = default;

		virtual std::stringstream& source() = 0;
		virtual std::stringstream& sink() = 0;
	};

	struct Handler
	{
		virtual ~Handler() = default;

		virtual void notify(Connection& connection, StreamsProvider& streams) = 0;
		virtual void notify(Handler& handler, Connection& connection, StreamsProvider& streams) = 0;

		virtual bool wants_more_bytes() = 0;
		virtual bool wants_to_write() = 0;
		virtual bool wants_to_live() = 0;

	protected:
		// StateManager<ParseResult> state_manager{ ParseResult::P_STARTING, ParseResult::P_ERROR, 1024 };
	};

	struct Connection
	{
		virtual ~Connection() = default;
		virtual void notify(ServerDirective directive) = 0;
		virtual void notify(Handler& handler) = 0;
		virtual void set_handler(std::shared_ptr<Handler>) = 0;
		virtual void halt() = 0;
		virtual ConnectionState get_state() = 0;
		virtual const int get_id() const = 0;
	};
};

using connection_ptr = std::shared_ptr<nimlib::Server::Types::Connection>;
using socket_ptr = std::unique_ptr<nimlib::Server::Types::Socket>;
using connection_id = int;
using headers_t = std::unordered_map<std::string, std::vector<std::string>>;
using header_validator = const std::function<bool(const std::string& value, const std::unordered_map<std::string, std::vector<std::string>>& headers)>;
