#include <iostream>
#include <format>
#include <span>
#include <string_view>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include <iostream>

#include "tcp_socket.h"
#include "logger/factory.h"

namespace nimlib::Server::Sockets
{
    TcpSocket::TcpSocket(int tcp_socket, const std::string& port) :
        tcp_socket_descriptor(tcp_socket),
        port{ port },
        log_agent{ nimlib::Server::Logging::Factory::get_agent("socket") }
    {}

    TcpSocket::TcpSocket(const std::string& port) :
        port{ port },
        log_agent{ nimlib::Server::Logging::Factory::get_agent("socket") }
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
            log_agent->error(std::format("failed to create socket to bind to port {}", port));
        }
        else
        {
            log_agent->info(std::format("socket {} created to bind to port {}", tcp_socket_descriptor, port));
        }

        // is there any reason to keep bind_address? apparently it can be freed after binding.
    }

    TcpSocket::~TcpSocket()
    {
        log_agent->info(std::format("closing socket {} bound to port", tcp_socket_descriptor, port));
        freeaddrinfo(bind_address);
        close(tcp_socket_descriptor);
    }

    const int TcpSocket::get_tcp_socket_descriptor() const { return tcp_socket_descriptor; }

    void TcpSocket::tcp_connect(const std::string& addr, const std::string& port)
    {
        struct addrinfo hints;
        memset(&hints, 0, sizeof(hints));
        hints.ai_socktype = SOCK_STREAM;
        struct addrinfo* peer_address;

        if (getaddrinfo(addr.data(), port.data(), &hints, &peer_address))
        {
            log_agent->error(std::format("getaddrinfo() failed on call to connect() for TCP connection to {}:{}", addr, port));
        }

        // printf("Remote address is: ");
        // char address_buffer[100];
        // char service_buffer[100];
        // getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
        //     address_buffer, sizeof(address_buffer),
        //     service_buffer, sizeof(service_buffer),
        //     NI_NUMERICHOST);
        // printf("%s %s\n", address_buffer, service_buffer);

        if (connect(tcp_socket_descriptor, peer_address->ai_addr, peer_address->ai_addrlen) == -1)
        {
            log_agent->error(std::format("call to connect() failed for TCP connection to {}:{}", addr, port));
        }
        else
        {
            log_agent->info(std::format("TCP connection established to {}:{}", addr, port));
        }

        freeaddrinfo(peer_address);
    }

    void TcpSocket::tcp_bind()
    {
        // TODO: return true or false depending on failing to successding
        if (bind_address)
        {
            auto bind_result = bind(
                tcp_socket_descriptor,
                bind_address->ai_addr,
                bind_address->ai_addrlen
            );

            if (bind_result != 0)
            {
                log_agent->error(std::format("failed to bind socket {} to port {}", tcp_socket_descriptor, port));
            }
            else
            {
                log_agent->info(std::format("bound socket {} to port {}", tcp_socket_descriptor, port));
            }
        }
    }

    void TcpSocket::tcp_listen()
    {
        if (listen(tcp_socket_descriptor, MAX_CONNECTIONS) < 0)
        {
            log_agent->error(std::format("socket {} failed to start listening on port {}", tcp_socket_descriptor, port));
        }
        else
        {
            log_agent->info(std::format("socket {} listening on port {}", tcp_socket_descriptor, port));
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
            log_agent->error(std::format("TCP socket {} failed to accept new connection", tcp_socket_descriptor));
            return {};
        }
        else
        {
            // log new connection (use page 93 of c network programming)
            std::string host_name;
            tcp_get_host_name(reinterpret_cast<sockaddr&>(client_address), host_name);
            log_agent->info(std::format("TCP socket {} accepted connection from {}", tcp_socket_descriptor, host_name));
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
        int bytes_count = send(tcp_socket_descriptor, buffer.data(), buffer.size(), 0);
        log_agent->info(std::format("read {} bytes from socket {}", bytes_count, tcp_socket_descriptor));
        return bytes_count;
    }

    int TcpSocket::tcp_send(std::string_view buffer)
    {
        int bytes_count = send(tcp_socket_descriptor, buffer.data(), buffer.size(), 0);
        log_agent->info(std::format("wrote {} bytes to socket {}", bytes_count, tcp_socket_descriptor));
        return bytes_count;
    }

    void TcpSocket::tcp_close()
    {
        log_agent->info(std::format("TCP socket {} closing", tcp_socket_descriptor));
        close(tcp_socket_descriptor);
    }
}
