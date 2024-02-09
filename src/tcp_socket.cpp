#include <iostream>
#include <span>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
// #include "errno.h"

#include "tcp_socket.h"

TcpSocket::TcpSocket(const std::string& port)
{
    addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(0, port.data(), &hints, &bind_address);

    tcp_socket_descriptor = socket(
        bind_address->ai_family,
        bind_address->ai_socktype,
        bind_address->ai_protocol
    );

    if (tcp_socket_descriptor < 0)
    {
        // log failure.
        std::cout << "failed to create socket" << std::endl;
    }

    // is there any reason to keep bind_address? apparently it can be freed after binding.
}

TcpSocket::TcpSocket(int tcp_socket) : tcp_socket_descriptor(tcp_socket) {}

TcpSocket::~TcpSocket()
{
    freeaddrinfo(bind_address);
    close(tcp_socket_descriptor);
}

const int TcpSocket::get_tcp_socket_descriptor() const { return tcp_socket_descriptor; }

void TcpSocket::tcp_bind()
{
    if (bind_address)
    {
        auto bind_result = bind(
            tcp_socket_descriptor,
            bind_address->ai_addr,
            bind_address->ai_addrlen
        );

        if (bind_result != 0)
        {
            // log failure.
            std::cout << "failed to bind socket to port" << std::endl;
        }
    }
}

void TcpSocket::tcp_listen()
{
    if (listen(tcp_socket_descriptor, MAX_CONNECTIONS) < 0)
    {
        // log failure.
        std::cout << "failed to listen" << std::endl;
    }
}

std::unique_ptr<TcpSocket> TcpSocket::tcp_accept()
{
    sockaddr_storage client_address{};
    socklen_t client_len = sizeof(client_address);
    int socket_client = accept(
        tcp_socket_descriptor,
        (sockaddr*)&client_address,
        &client_len
    );

    if (socket_client < 0)
    {
        // log failure.
        return {};
    }
    else
    {
        // log new connection (use page 93 of c network programming)
        std::string host_name;
        tcp_get_host_name(reinterpret_cast<sockaddr&>(client_address), host_name);
        std::cout << host_name << std::endl;
        return std::make_unique<TcpSocket>(socket_client);
    }
}

void TcpSocket::tcp_get_host_name(const sockaddr& socket_address, std::string& host_name)
{
    char name_buffer[NI_MAXHOST], service_buffer[NI_MAXSERV];
    getnameinfo(
        &socket_address,
        sizeof(socket_address),
        name_buffer,
        sizeof(name_buffer),
        service_buffer,
        sizeof(service_buffer),
        NI_NUMERICHOST | NI_NUMERICSERV
    );
    host_name = std::string(name_buffer);
}

int TcpSocket::tcp_read(std::span<char> buffer, int flags = 0)
{
    return recv(tcp_socket_descriptor, buffer.data(), buffer.size(), flags);
}

int TcpSocket::tcp_send(std::span<char> buffer)
{
    return send(tcp_socket_descriptor, buffer.data(), buffer.size(), 0);
}

void TcpSocket::tcp_close()
{
    close(tcp_socket_descriptor);
}
