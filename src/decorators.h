#pragma once

#include <memory>
#include <concepts>

#include "polling_server.h"
#include "polling_server_with_logger.h"
#include "tcp_socket.h"
#include "tcp_socket_with_logger.h"

namespace nimlib::Server::Decorators
{
    using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Sockets::TcpSocketWithLogger;
    using nimlib::Server::PollingServer;
    using nimlib::Server::PollingServerWithLogger;

    template <typename T>
    concept IsTcpSocket = std::derived_from<T, TcpSocketInterface>;

    template <IsTcpSocket T = TcpSocketWithLogger>
    std::unique_ptr<TcpSocketInterface> decorate(std::unique_ptr<TcpSocketInterface> socket)
    {
        return std::make_unique<T>(std::move(socket));
    }
};
