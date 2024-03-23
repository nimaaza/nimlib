#pragma once

#include <queue>
#include <memory>
#include <poll.h>

#include "types.h"

namespace nimlib::Server
{
    using nimlib::Server::Types::PollingServerInterface;
    using nimlib::Server::Types::ConnectionInterface;
    using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Constants::ConnectionState;
    using connection_ptr = std::unique_ptr<ConnectionInterface>;

    class PollingServer : public PollingServerInterface
    {
    public:
        PollingServer(const std::string&);
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
        bool allowed_to_read(ConnectionState state) const;
        bool allowed_to_write(ConnectionState state) const;

    private:
        const std::string& port;
        std::unique_ptr<TcpSocketInterface> server_socket;
        std::vector<connection_ptr> connections{};
        pollfd server_fd{};
    };
};
