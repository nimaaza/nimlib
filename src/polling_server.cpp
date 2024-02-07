#include "polling_server.h"

PollingServer::PollingServer(const std::string& port)
    : port{ port },
    connections{},
    read_queue{},
    write_queue{},
    server_socket{ std::make_unique<TcpSocket>(port) }
{
    server_socket->tcp_bind();
    server_socket->tcp_listen();
}

PollingServer::~PollingServer() {}

void PollingServer::run()
{
    std::vector<pollfd> sockets;
    int poll_result;
    std::vector allowed_states_for_read{ STARTING, PENDING };
    std::vector allowed_states_for_write{ WRITING };

    while (true)
    {
        // setup fds
        setup_fds(sockets);

        // poll;
        poll_result = poll(sockets.data(), sockets.size(), -1);

        // accept and create new connection
        accept_new_connection(sockets);

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

void PollingServer::setup_fds(std::vector<pollfd>& sockets)
{
    static const int server_socket_descriptor{ server_socket->get_tcp_socket_descriptor() };

    sockets.clear();
    pollfd server_fd;
    create_pollfds_entry(server_socket_descriptor, server_fd);
    sockets.push_back(server_fd);

    for (auto& c : connections)
    {
        pollfd fds;
        create_pollfds_entry(c.first, fds);
        sockets.push_back(fds);
    }
}

void PollingServer::create_pollfds_entry(int s, pollfd& fds)
{
    fds.fd = s;
    fds.events = POLLIN | POLLOUT;
    fds.revents = 0;
}

void PollingServer::accept_new_connection(std::vector<pollfd>& sockets)
{
    if (sockets[0].revents & POLLIN)
    {
        auto connection_socket = server_socket->tcp_accept();
        auto connection = std::make_unique<Connection>(nullptr, std::move(connection_socket));
        connections.emplace(connection->get_tcp_socket_descriptor(), std::move(connection));
    }

    sockets.erase(sockets.begin());
}

void PollingServer::queue_connections(
    short poll_event,
    std::vector<pollfd>& sockets,
    std::vector<ConnectionState>& allowed_states,
    std::queue<connection_ptr>& queue
)
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
