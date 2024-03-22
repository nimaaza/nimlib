#pragma once

#include <span>
#include <netdb.h>

#include "tcp_socket.h"
#include "logger/factory.h"

namespace nimlib::Server::Sockets
{
    class TcpSocketWithLogger : public TcpSocketInterface
    {
    public:
        explicit TcpSocketWithLogger(std::unique_ptr<TcpSocketInterface> tcp_socket);
        ~TcpSocketWithLogger();

        TcpSocketWithLogger(const TcpSocketWithLogger&) = delete;
        TcpSocketWithLogger& operator=(const TcpSocketWithLogger&) = delete;
        TcpSocketWithLogger(TcpSocketWithLogger&&) noexcept;
        TcpSocketWithLogger& operator=(TcpSocketWithLogger&&) noexcept;

        // int tcp_connect(const std::string& addr, const std::string& port) override;
        int tcp_bind() override;
        int tcp_listen() override;
        std::unique_ptr<TcpSocketInterface> tcp_accept() override;
        void tcp_get_host_name(const sockaddr& socket_address, std::string& host_name) override;
        int tcp_read(std::span<uint8_t> buffer, int flags) override;
        int tcp_send(std::span<uint8_t> buffer) override;
        int tcp_send(std::string_view buffer) override;
        void tcp_close() override;  // TODO: may never be used
        const int get_tcp_socket_descriptor() const override;
        const std::string& get_port() const override;

    private:
        std::unique_ptr<TcpSocketInterface> tcp_socket;
        std::shared_ptr<nimlib::Server::Logging::LoggerAgent> log_agent;
    };
};
