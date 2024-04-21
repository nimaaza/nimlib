#pragma once

#include <queue>
#include <memory>
#include <poll.h>

#include "common/types.h"
#include "tcp_connection_pool.h"

namespace nimlib::Server
{
    using nimlib::Server::Types::Server;
    using nimlib::Server::Types::Connection;
    using nimlib::Server::Types::Socket;
    using nimlib::Server::Constants::ServerDirective;
    using nimlib::Server::Constants::ConnectionState;

    class PollingServer : public Server
    {
    public:
        PollingServer(const std::string& port);
        ~PollingServer();

        PollingServer(const PollingServer&) = delete;
        PollingServer& operator=(const PollingServer&) = delete;
        PollingServer(PollingServer&&) noexcept = delete;
        PollingServer& operator=(PollingServer&&) noexcept = delete;

        void run() override;

    private:
        void setup_fds(std::vector<pollfd>&);
        void accept_new_connection(std::vector<pollfd>&);
        void handle_connections(std::vector<pollfd>&);

        static void create_pollfds_entry(int, pollfd&);
        static bool allowed_to_read(ConnectionState state);
        static bool allowed_to_write(ConnectionState state);

    private:
        const std::string& port;
        std::unique_ptr<Socket> server_socket;
        TcpConnectionPool& connection_pool;
        pollfd server_fd{};
    };
};
