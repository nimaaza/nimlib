#include "../../src/tcp_socket.h"

#include <span>
#include <cstdlib>
#include <ctime>

namespace nimlib::Server::Sockets
{
    TcpSocket::TcpSocket(int tcp_socket, const std::string& port) : TcpSocketInterface{ port, tcp_socket } {}

    TcpSocket::TcpSocket(const std::string& port) : TcpSocketInterface{ port , 0 } {}

    TcpSocket::~TcpSocket() {}

    // int TcpSocket::tcp_connect(const std::string& addr, const std::string& port) { return 0; }

    int TcpSocket::tcp_bind() { return 0; }

    int TcpSocket::tcp_listen() { return 1; }

    std::unique_ptr<TcpSocketInterface> TcpSocket::tcp_accept()
    {
        static int last_accepted_socket{ 10 };
        last_accepted_socket += 1;
        return std::make_unique<TcpSocket>(last_accepted_socket);
    }

    void TcpSocket::tcp_get_host_name(const sockaddr& socket_address, std::string& host_name) { host_name = "hostname"; }

    int TcpSocket::tcp_read(std::span<uint8_t> buffer, int flags)
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
            i++;
            total_socket_read_cout++;

            if (total_socket_read_cout >= MAX_BYTES_TO_READ) break;
        }

        return i;
    }

    int TcpSocket::tcp_send(std::span<uint8_t> buffer)
    {
        const static int MAX_BYTES_TO_WRITE_IN_ONE_CALL{ 10'000 };
        static int total_socket_write_count{ 0 };

        if (total_socket_write_count > MAX_BYTES_TO_WRITE_IN_ONE_CALL * 4) return -1;

        int bytes_sent = buffer.size() <= MAX_BYTES_TO_WRITE_IN_ONE_CALL
            ? buffer.size()
            : MAX_BYTES_TO_WRITE_IN_ONE_CALL;

        total_socket_write_count += bytes_sent;

        return bytes_sent;
    }

    int TcpSocket::tcp_send(std::string_view buffer)
    {
        // TODO: find better way to pass string_view as span
        std::vector<uint8_t> s{};
        for (auto c : buffer) s.push_back(c);
        return tcp_send(s);
    }

    void TcpSocket::tcp_close() {}

    const int TcpSocket::get_tcp_socket_descriptor() const { return tcp_socket_descriptor; }

    const std::string& TcpSocket::get_port() const { return port; }
}
