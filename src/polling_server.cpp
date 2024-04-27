#include <string>
#include <cassert>

#include "polling_server.h"
#include "tcp_socket.h"

#ifdef NO_TESTING
#include "common/decorators.h"
#else
#include "../tests/support/decorators.h"
#endif

using nimlib::Server::Sockets::TcpSocket;

namespace nimlib::Server
{
	PollingServer::PollingServer(const std::string& port)
		: port{ port },
		server_socket{ nimlib::Server::Decorators::decorate(std::make_unique<TcpSocket>(port)) },
		connection_pool{ nimlib::Server::TcpConnectionPool::get_pool() }
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
			poll_result = poll(sockets.data(), sockets.size(), 10); // TODO: timeout for polling
			accept_new_connection(sockets);
			handle_connections(sockets);
			connection_pool.clean_up();
		}
	}

	void PollingServer::setup_fds(std::vector<pollfd>& sockets)
	{
		sockets.clear();
		sockets.push_back(server_fd);

		for (const auto& connection : connection_pool.get_all())
		{
			if (connection->get_state() != ConnectionState::INACTIVE)
			{
				assert(connection->get_id() > 0);
				pollfd fds{};
				create_pollfds_entry(connection->get_id(), fds);
				sockets.push_back(fds);
			}
		}

		// Server socket must always be the first socket in the vector.
		assert(sockets[0].fd == server_socket->get_tcp_socket_descriptor());
	}

	void PollingServer::accept_new_connection(std::vector<pollfd>& sockets)
	{
		if (sockets[0].revents & POLLIN)
		{
			auto accepted_socket = nimlib::Server::Decorators::decorate(server_socket->tcp_accept());
			connection_pool.record_connection(std::move(accepted_socket));
		}
	}

	void PollingServer::handle_connections(std::vector<pollfd>& sockets)
	{
		std::for_each(sockets.begin() + 1, sockets.end(), [&](const auto& socket) {
			auto connection = connection_pool.find(socket.fd);
			auto state = connection->get_state();
			if (socket.revents & POLLIN)
			{
				if (allowed_to_read(state)) connection->notify(ServerDirective::READ_SOCKET);
			}
			else if (socket.revents & POLLOUT)
			{
				if (allowed_to_write(state)) connection->notify(ServerDirective::WRITE_SOCKET);
			}
			else
			{
				// TODO: socket might be in a state which we don't handle?
			}
        });
	}

	void PollingServer::create_pollfds_entry(int socket, pollfd& fds)
	{
		fds.fd = socket;
		fds.events = POLLIN | POLLOUT;
		fds.revents = 0;
	}

	bool PollingServer::allowed_to_read(ConnectionState state)
	{
		return (state == ConnectionState::READY_TO_READ);
	}

	bool PollingServer::allowed_to_write(ConnectionState state)
	{
		return state == ConnectionState::READY_TO_WRITE;
	}
}
