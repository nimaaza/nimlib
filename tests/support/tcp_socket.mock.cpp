#include "../../src/tcp_socket.h"

#include <span>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include "tcp_socket.mock.h"

namespace nimlib::Server::Sockets
{
    MockTcpSocket::MockTcpSocket(
        int tcp_socket,
        int max_bytes_to_read,
        int max_bytes_to_write
    )
        : Socket{ "", tcp_socket },
        max_bytes_to_read{ max_bytes_to_read },
        max_bytes_to_write{ max_bytes_to_write }
    {
        std::srand(std::time(nullptr));
        for (int i = 0; i < max_bytes_to_read; i++)
        {
            read_buffer += 'a' + rand() % 26;;
        }
    }

    MockTcpSocket::~MockTcpSocket() {}

    // int MockTcpSocket::tcp_connect(const std::string& addr, const std::string& port) { return 0; }

    int MockTcpSocket::tcp_bind() { return 0; }

    int MockTcpSocket::tcp_listen() { return 1; }

    std::unique_ptr<Socket> MockTcpSocket::tcp_accept()
    {
        static int last_accepted_socket{ 10 };
        last_accepted_socket += 1;
        return std::make_unique<MockTcpSocket>(last_accepted_socket, max_bytes_to_read, max_bytes_to_write);
    }

    void MockTcpSocket::tcp_get_host_name(const sockaddr& socket_address, std::string& host_name) { host_name = "hostname"; }

    int MockTcpSocket::tcp_read(std::span<uint8_t> buffer, int flags)
    {
        if (total_socket_read_count >= max_bytes_to_read)
        {
            read_sequence++;
            return -1;
        }

        size_t bytes = byte_counts_to_read.size() > read_sequence
            ? byte_counts_to_read[read_sequence]
            : buffer.size();

        int i{ 0 };
        while (i < buffer.size() && i < bytes)
        {
            buffer[i] = read_buffer[i];
            read_result << read_buffer[i];
            i++;
            total_socket_read_count++;

            if (total_socket_read_count >= max_bytes_to_read) break;
        }

        read_sequence++;
        return i;
    }

    int MockTcpSocket::tcp_send(std::span<uint8_t> buffer)
    {
        if (total_socket_write_count > max_bytes_to_write * 4) return -1;

        int bytes_to_send = buffer.size() <= max_bytes_to_write
            ? buffer.size()
            : max_bytes_to_write;

        for (int i = 0; i < bytes_to_send; i++)
        {
            write_result << buffer[i];
        }

        total_socket_write_count += bytes_to_send;

        return bytes_to_send;
    }

    int MockTcpSocket::tcp_send(std::string_view buffer)
    {
        // TODO: find better way to pass string_view as span
        std::vector<uint8_t> s{};
        for (auto c : buffer) s.push_back(c);
        return tcp_send(s);
    }

    void MockTcpSocket::tcp_close() {}

    const int MockTcpSocket::get_tcp_socket_descriptor() const { return tcp_socket_descriptor; }

    const std::string& MockTcpSocket::get_port() const { return port; }
}
