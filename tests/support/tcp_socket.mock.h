#pragma once

#include <sstream>
#include <string>

#include "../src/types.h"

namespace nimlib::Server::Sockets
{
    struct MockTcpSocket : public TcpSocketInterface
    {
        MockTcpSocket(int tcp_socket, const std::string& port = "");
        MockTcpSocket(const std::string& port);
        ~MockTcpSocket();

        MockTcpSocket(const MockTcpSocket&) = delete;
        MockTcpSocket& operator=(const MockTcpSocket&) = delete;
        MockTcpSocket(MockTcpSocket&&) noexcept;
        MockTcpSocket& operator=(MockTcpSocket&&) noexcept;

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

        std::stringstream read_result{};
        std::stringstream write_result{};
    };
};
