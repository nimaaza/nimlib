#pragma once

#include <memory>
#include <concepts>

#include "../decorations/implementations.h"

namespace nimlib::Server::Decorators
{
    using nimlib::Server::Types::Socket;
    using nimlib::Server::Types::Server;
    using nimlib::Server::Sockets::TcpSocketWithLogger;
    using nimlib::Server::PollingServer;
    using nimlib::Server::PollingServerWithLogger;

    template <typename T>
    concept IsTcpSocket = std::derived_from<T, Socket>;

    template <typename T>
    concept IsPollingServer = std::derived_from<T, Server>;

    template <IsTcpSocket T = TcpSocketWithLogger>
    std::unique_ptr<Socket> decorate(std::unique_ptr<Socket> socket)
    {
        return std::make_unique<T>(std::move(socket));
    }

    template<IsPollingServer T = PollingServerWithLogger>
    std::unique_ptr<Server> decorate(std::unique_ptr<Server> polling_server)
    {
        return std::make_unique<T>(std::move(polling_server));
    }
};
