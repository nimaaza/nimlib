#include <unordered_map>
#include <string>
#include <algorithm>
#include <cassert>

#include "polling_server.h"
#include "connection.h"

#ifdef NO_TESTING
#include "decorators.h"
#else
#include "../tests/support/decorators.h"
#endif

namespace nimlib::Server
{
	PollingServer::PollingServer(const std::string& port)
		: port{ port },
		server_socket{ nimlib::Server::Decorators::decorate(std::make_unique<TcpSocket>(port)) }
	{
		server_socket->tcp_bind();
		server_socket->tcp_listen();
		create_pollfds_entry(server_socket->get_tcp_socket_descriptor(), server_fd);
	}

	PollingServer::~PollingServer()
	{
		server_socket->tcp_close();
	}

	void PollingServer::run()
	{
		std::vector<pollfd> sockets;
		int poll_result{};

		while (true)
		{
			setup_fds(sockets);
			poll_result = poll(sockets.data(), sockets.size(), -1); // TODO: timeout for polling
			accept_new_connection(sockets);
			handle_connections(sockets);

			// Filter & clear out connections (done or in error).
			std::for_each(connections.begin(), connections.end(), [](auto& connection) {
				if (connection)
				{
					auto [state, _] = connection->get_state();
					if (state == ConnectionState::DONE || state == ConnectionState::CON_ERROR)
					{
						connection.reset();
					}
				}
			});
		}
	}

	void PollingServer::setup_fds(std::vector<pollfd>& sockets)
	{
		sockets.clear();
		sockets.push_back(server_fd);

		for (const auto& connection : connections)
		{
			if (connection)
			{
				pollfd fds{};
				create_pollfds_entry(connection->get_id(), fds);
				sockets.push_back(fds);
			}
		}

		// Server socket must always be the first socket in the vector.
		assert(sockets[0].fd == server_socket->get_tcp_socket_descriptor());
	}

	void PollingServer::create_pollfds_entry(int socket, pollfd& fds)
	{
		fds.fd = socket;
		fds.events = POLLIN | POLLOUT;
		fds.revents = 0;
	}

	void PollingServer::accept_new_connection(std::vector<pollfd>& sockets)
	{
		if (sockets[0].revents & POLLIN)
		{
			auto accepted_socket = nimlib::Server::Decorators::decorate(server_socket->tcp_accept());
			connection_id id = accepted_socket->get_tcp_socket_descriptor();
			auto socket_wrapper = std::make_unique<TcpSocketAdapter>(std::move(accepted_socket));
			auto connection = std::make_unique<Connection>(std::move(socket_wrapper), id);
			if (connections.size() < id) connections.resize(id + 1);
			connections[id] = std::move(connection);
		}
	}

	void PollingServer::handle_connections(std::vector<pollfd>& sockets)
	{
		std::for_each(sockets.begin() + 1, sockets.end(), [&](const auto& socket) {
			if (socket.revents & POLLIN)
			{
				auto [state, _] = connections[socket.fd]->get_state();
				if (allowed_to_read(state))
				{
					connections[socket.fd]->read();
				}
			}
			else if (socket.revents & POLLOUT)
			{
				auto [state, _] = connections[socket.fd]->get_state();
				if (allowed_to_write(state))
				{
					connections[socket.fd]->write();
				}
			}
		});
	}

	bool PollingServer::allowed_to_read(ConnectionState state) const
	{
		return (state == ConnectionState::STARTING
			|| state == ConnectionState::READING
			|| state == ConnectionState::PENDING);
	}

	bool PollingServer::allowed_to_write(ConnectionState state) const
	{
		return state == ConnectionState::WRITING;
	}
}
