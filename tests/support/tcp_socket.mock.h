#pragma once

#include <sstream>
#include <string>

#include "../../src/common/types.h"

namespace nimlib::Server::Sockets
{
    struct MockTcpSocket : public TcpSocketInterface
    {
        MockTcpSocket(int tcp_socket, int max_bytes_to_read = 1024, int max_bytes_to_write = 1024);
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

        std::string read_buffer{};
        std::stringstream read_result{};
        std::stringstream write_result{};
        int total_socket_read_count{ 0 };
        int total_socket_write_count{ 0 };
        // How many times the tcp_read() method was called.
        int read_sequence{ 0 };
        // Reading and writing more bytes than the values below will cause
        // the socket to return an error code. This is used to mimick when
        // the socket fails.
        const int max_bytes_to_read;
        const int max_bytes_to_write;
        // The vector below can be used to mimick the scenario of reading
        // different bytes count on each call to the tcp_read method.
        std::vector<int> byte_counts_to_read{};
    };
};
