#include "../../src/tcp_socket.h"

#include <span>
#include <sstream>
#include <cstdlib>
#include <ctime>

#include "tcp_socket.mock.h"

namespace nimlib::Server::Sockets
{
    MockTcpSocket::MockTcpSocket(int tcp_socket, const std::string& port) : TcpSocketInterface{ port, tcp_socket } {}

    MockTcpSocket::MockTcpSocket(const std::string& port) : TcpSocketInterface{ port , 0 } {}

    MockTcpSocket::~MockTcpSocket() {}

    // int MockTcpSocket::tcp_connect(const std::string& addr, const std::string& port) { return 0; }

    int MockTcpSocket::tcp_bind() { return 0; }

    int MockTcpSocket::tcp_listen() { return 1; }

    std::unique_ptr<TcpSocketInterface> MockTcpSocket::tcp_accept()
    {
        static int last_accepted_socket{ 10 };
        last_accepted_socket += 1;
        return std::make_unique<MockTcpSocket>(last_accepted_socket);
    }

    void MockTcpSocket::tcp_get_host_name(const sockaddr& socket_address, std::string& host_name) { host_name = "hostname"; }

    int MockTcpSocket::tcp_read(std::span<uint8_t> buffer, int flags)
    {
        const static int MAX_BYTES_TO_READ{ 1000 };
        static int total_socket_read_cout{ 0 };

        std::srand(std::time(nullptr));

        if (total_socket_read_cout >= MAX_BYTES_TO_READ) return -1;

        int i{ 0 };
        while (i < buffer.size())
        {
            uint8_t rand_char = 'a' + rand() % 26;
            buffer[i] = rand_char;
            read_result << rand_char;
            i++;
            total_socket_read_cout++;

            if (total_socket_read_cout >= MAX_BYTES_TO_READ) break;
        }

        return i;
    }

    int MockTcpSocket::tcp_send(std::span<uint8_t> buffer)
    {
        const static int MAX_BYTES_TO_WRITE_IN_ONE_CALL{ 10'000 };
        static int total_socket_write_count{ 0 };

        if (total_socket_write_count > MAX_BYTES_TO_WRITE_IN_ONE_CALL * 4) return -1;

        int bytes_to_send = buffer.size() <= MAX_BYTES_TO_WRITE_IN_ONE_CALL
            ? buffer.size()
            : MAX_BYTES_TO_WRITE_IN_ONE_CALL;

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
