#pragma once

#include <queue>
#include <memory>
#include <poll.h>

#include "common.h"
#include "connection.h"
#include "tcp_socket.h"
#include "socket.h"
#include "logger/factory.h"

using ConnectionState = nimlib::Server::Constants::ConnectionState;

class PollingServer
{
    using connection_ptr = std::unique_ptr<Connection>;

public:
    PollingServer(const std::string&);
    ~PollingServer();

    PollingServer(const PollingServer&) = delete;
    PollingServer& operator=(const PollingServer&) = delete;

    PollingServer(PollingServer&&) noexcept = delete;
    PollingServer& operator=(PollingServer&&) noexcept = delete;

    void run();

private:
    void setup_fds(std::vector<pollfd>&);
    void create_pollfds_entry(int, pollfd&);
    void accept_new_connection(std::vector<pollfd>&);
    void queue_connections(short, std::vector<pollfd>&, const std::vector<ConnectionState>&, std::queue<connection_ptr>&);
    void handle_queue(std::queue<connection_ptr>&, const std::vector<ConnectionState>&, bool);

private:
    const std::string& port;
    std::unique_ptr<TcpSocket> server_socket;
    pollfd server_fd;
    std::unordered_map<int, connection_ptr> connections;
    std::queue<connection_ptr> read_queue;
    std::queue<connection_ptr> write_queue;
    std::shared_ptr<nimlib::Server::Logging::LoggerAgent> main_log_agent;

    const std::vector<ConnectionState> allowed_states_for_read{
        ConnectionState::STARTING,
        ConnectionState::READING,
        ConnectionState::PENDING
    };
    const std::vector<ConnectionState> allowed_states_for_write{ ConnectionState::WRITING };
};
