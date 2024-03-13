#include <unordered_map>
#include <string>
#include <algorithm>
#include <format>

#include "polling_server.h"
#include "metrics/factory.h"
#include "socket.h"
#include "version.h"

PollingServer::PollingServer(const std::string& port)
    : port{ port },
    connections{},
    read_queue{},
    write_queue{},
    main_log_agent{ nimlib::Logging::Factory::get_agent("main") },
    server_socket{ std::make_unique<TcpSocket>(port) }
{
    main_log_agent->log(std::format("server version {}.{}.{} starting", nimlib_VERSION_MAJOR, nimlib_VERSION_MINOR, nimlib_VERSION_PATCH));

    nimlib::Metrics::Factory<long>::instanciate_metric(TIME_TO_RESPONSE)
        .measure_avg()
        .measure_max()
        .get();

    server_socket->tcp_bind();
    server_socket->tcp_listen();
    create_pollfds_entry(server_socket->get_tcp_socket_descriptor(), server_fd);

    main_log_agent->log(std::format("listening on port: {}", port));
}

PollingServer::~PollingServer()
{
    main_log_agent->log(std::format("server shutting down", port));
    server_socket->tcp_close();
}

void PollingServer::run()
{
    std::vector<pollfd> sockets;
    int poll_result{};

    while (true)
    {
        // setup fds
        setup_fds(sockets);

        // poll;
        poll_result = poll(sockets.data(), sockets.size(), -1); // TODO: timeout for polling

        // accept and create new connection
        accept_new_connection(sockets);

        // move connections to reading & writing queue
        queue_connections(POLLIN, sockets, allowed_states_for_read, read_queue);
        queue_connections(POLLOUT, sockets, allowed_states_for_write, write_queue);

        // handle reads & writes
        handle_queue(read_queue, allowed_states_for_read, true);
        handle_queue(write_queue, allowed_states_for_write, false);

        // filter connections (moving remaining live connections to connections map)
        std::erase_if(connections, [](const auto& c) {
            auto [state, elapsed] = c.second->get_state();
            return (state == DONE || state == CON_ERROR);
            });
    }
}

void PollingServer::setup_fds(std::vector<pollfd>& sockets)
{
    sockets.clear();
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
        auto new_connection = server_socket->tcp_accept();
        connection_id id = new_connection->get_tcp_socket_descriptor();
        auto socket_wrapper = std::make_unique<TcpSocketAdapter>(std::move(new_connection));
        auto connection = std::make_unique<Connection>(std::move(socket_wrapper), id);
        connections.emplace(id, std::move(connection));
    }
}

void PollingServer::queue_connections(
    short poll_event,
    std::vector<pollfd>& sockets,
    const std::vector<ConnectionState>& allowed_states,
    std::queue<connection_ptr>& queue)
{
    std::for_each(sockets.begin() + 1, sockets.end(), [&](const auto& s) {
        if ((s.revents & poll_event) && (connections.contains(s.fd)))
        {
            auto state = connections[s.fd]->get_state().first;
            if (std::find(allowed_states.begin(), allowed_states.end(), state) != allowed_states.end())
            {
                auto connection = std::move(connections[s.fd]);
                connections.erase(s.fd);
                queue.emplace(std::move(connection));
            }
        }});
}

void PollingServer::handle_queue(
    std::queue<connection_ptr>& queue,
    const std::vector<ConnectionState>& allowed_states,
    bool read_buff)
{
    while (!queue.empty())
    {
        auto state = queue.front()->get_state().first;
        if (std::find(allowed_states.begin(), allowed_states.end(), state) != allowed_states.end())
        {
            read_buff ? queue.front()->read() : queue.front()->write();
        }
        auto s = queue.front()->get_id();
        connections.emplace(s, std::move(queue.front()));
        queue.pop();
    }
}
