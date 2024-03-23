#pragma once

#include <queue>
#include <memory>
#include <poll.h>

#include "types.h"
#include "connection_pool.h"

namespace nimlib::Server
{
    using nimlib::Server::Types::PollingServerInterface;
    using nimlib::Server::Types::ConnectionInterface;
    using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Constants::ConnectionState;

    class PollingServer : public PollingServerInterface
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
        void create_pollfds_entry(int, pollfd&);
        void accept_new_connection(std::vector<pollfd>&);
        void handle_connections(std::vector<pollfd>& socket);
        void clear_connections();
        bool allowed_to_read(ConnectionState state) const;
        bool allowed_to_write(ConnectionState state) const;

    private:
        const std::string& port;
        std::unique_ptr<TcpSocketInterface> server_socket;
        ConnectionPool& connection_pool;
        pollfd server_fd{};
    };
};
