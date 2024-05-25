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
	using nimlib::Server::Constants::HandlerState;

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

		virtual bool wants_more_bytes() = 0;       // HandlerState::INCOMPLETE_INPUT
		virtual bool wants_to_write() = 0;         // HandlerState::FINISHED_NO_WAIT
		virtual bool wants_to_live() = 0;          // HandlerState::FINISHED_WAIT
		virtual bool wants_to_be_calledback() = 0; // HandlerState::RECALL

		virtual HandlerState get_state() = 0;

	protected:
		inline static const std::unordered_map<HandlerState, std::vector<HandlerState>> transition_map
		{
			{HandlerState::READY_TO_HANDLE, {HandlerState::H_HANDLING}},
			{HandlerState::H_HANDLING, {HandlerState::FINISHED_WAIT, HandlerState::FINISHED_NO_WAIT, HandlerState::RECALL, HandlerState::INCOMPLETE_INPUT}},
			{HandlerState::INCOMPLETE_INPUT, {HandlerState::FINISHED_WAIT, HandlerState::FINISHED_NO_WAIT, HandlerState::RECALL}},
			{HandlerState::FINISHED_NO_WAIT, {HandlerState::READY_TO_HANDLE}},
			{HandlerState::FINISHED_WAIT, {HandlerState::READY_TO_HANDLE}},
			{HandlerState::RECALL, {HandlerState::FINISHED_WAIT, HandlerState::FINISHED_NO_WAIT}},
		};
		inline static const std::unordered_map<HandlerState, int> max_reset_counts{};
		inline static const std::unordered_map<HandlerState, long> time_outs{};
		nimlib::Server::Utils::StateManager<HandlerState> state_manager{
			HandlerState::READY_TO_HANDLE,
			HandlerState::HANDLER_ERROR,
			transition_map,
			max_reset_counts,
			time_outs
		};
	};

	struct Connection
	{
		virtual ~Connection() = default;
		virtual void accept_socket(std::unique_ptr<Socket> s) = 0;
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
