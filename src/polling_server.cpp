#include "polling_server.h"

PollingServer::PollingServer(const std::string& port)
    : port{ port },
    connections{},
    read_queue{},
    write_queue{},
    server_socket{ std::make_unique<TcpSocket>(port) }
{}

PollingServer::~PollingServer() {}

void PollingServer::run()
{
    server_socket->tcp_bind();
    server_socket->tcp_listen();

    int poll_result;
    std::vector allowed_states_for_read{ STARTING, PENDING };
    std::vector allowed_states_for_write{ WRITING };
    std::vector<pollfd> sockets;

    while (true)
    {
        // setup fds
        sockets.clear();
        sockets.push_back(create_socket_descriptor(server_socket->get_tcp_socket_descriptor()));
        for (auto& c : connections)
        {
            sockets.push_back(create_socket_descriptor(c.first));
        }

        // poll;
        poll_result = poll(sockets.data(), sockets.size(), -1);

        // accept and create new connection
        if (sockets[0].revents & POLLIN)
        {
            auto connection_socket = server_socket->tcp_accept();
            auto s = connection_socket->get_tcp_socket_descriptor();
            auto connection = std::make_unique<Connection>(nullptr, std::move(connection_socket));
            connections.emplace(s, std::move(connection));
        }
        sockets.erase(sockets.begin());

        // move connections to reading & writing queue
        queue_connections(POLLIN, sockets, allowed_states_for_read, read_queue);
        queue_connections(POLLOUT, sockets, allowed_states_for_write, write_queue);

        // handle reads & writes
        handle_reads();
        handle_writes();

        // filter connections (moving remaining live connections to connections map)
        std::erase_if(connections, [](const auto& c) { return (c.second->get_state() == DONE); });
    }
}

pollfd PollingServer::create_socket_descriptor(int s)
{
    pollfd fds;
    fds.fd = s;
    fds.events = POLLIN | POLLOUT;
    fds.revents = 0;
    return fds;
}

void PollingServer::queue_connections(short poll_event, std::vector<pollfd>& sockets, std::vector<ConnectionState>& allowed_states, std::queue<std::unique_ptr<Connection>>& queue)
{
    for (auto& s : sockets)
    {
        if ((s.revents & poll_event) && (connections.contains(s.fd)))
        {
            auto ConnectionState = connections[s.fd]->get_state();
            if (std::find(allowed_states.begin(), allowed_states.end(), ConnectionState) != allowed_states.end())
            {
                auto connection = std::move(connections[s.fd]);
                connections.erase(s.fd);
                queue.emplace(std::move(connection));
            }
        }
    }
}

void PollingServer::handle_reads()
{
    while (!read_queue.empty())
    {
        auto state = read_queue.front()->get_state();
        if (state == PENDING || state == STARTING)
            read_queue.front()->read();
        auto s = read_queue.front()->get_tcp_socket_descriptor();
        connections.emplace(s, std::move(read_queue.front()));
        read_queue.pop();
    }
}

void PollingServer::handle_writes()
{
    while (!write_queue.empty())
    {
        auto state = write_queue.front()->get_state();
        if (state == WRITING)
            write_queue.front()->write();
        auto s = write_queue.front()->get_tcp_socket_descriptor();
        connections.emplace(s, std::move(write_queue.front()));
        write_queue.pop();
    }
}
