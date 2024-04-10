#pragma once

#include <memory>
#include <concepts>

#include "../decorations/implementations.h"

namespace nimlib::Server::Decorators
{
    using nimlib::Server::Types::TcpSocketInterface;
    using nimlib::Server::Types::PollingServerInterface;
    using nimlib::Server::Sockets::TcpSocketWithLogger;
    using nimlib::Server::PollingServer;
    using nimlib::Server::PollingServerWithLogger;

    template <typename T>
    concept IsTcpSocket = std::derived_from<T, TcpSocketInterface>;

    template <typename T>
    concept IsPollingServer = std::derived_from<T, PollingServerInterface>;

    template <IsTcpSocket T = TcpSocketWithLogger>
    std::unique_ptr<TcpSocketInterface> decorate(std::unique_ptr<TcpSocketInterface> socket)
    {
        return std::make_unique<T>(std::move(socket));
    }

    template<IsPollingServer T = PollingServerWithLogger>
    std::unique_ptr<PollingServerInterface> decorate(std::unique_ptr<PollingServerInterface> polling_server)
    {
        return std::make_unique<T>(std::move(polling_server));
    }
};
