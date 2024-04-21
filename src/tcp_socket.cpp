#include <cstring>
#include <span>
#include <string_view>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>

#include "tcp_socket.h"

namespace nimlib::Server::Sockets
{
    TcpSocket::TcpSocket(int tcp_socket, const std::string& port) :
        Socket{ port, tcp_socket }
    {
        // TODO: can I validate the socket instead of blindly accepting it?
    }

    TcpSocket::TcpSocket(const std::string& port) : Socket{ port, 0 }
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
            // log_agent->error(std::format("failed to create socket to bind to port {}", port));
        }
        else
        {
            // log_agent->info(std::format("socket {} created to bind to port {}", tcp_socket_descriptor, port));
        }

        // is there any reason to keep bind_address? apparently it can be freed after binding.
    }

    TcpSocket::~TcpSocket()
    {
        // freeaddrinfo(bind_address); // Commented out because address sanitizer complained.
        close(tcp_socket_descriptor);
    }

    //    int TcpSocket::tcp_connect(const std::string& addr, const std::string& port)
    //    {
    //        struct addrinfo hints;
    //        memset(&hints, 0, sizeof(hints));
    //        hints.ai_socktype = SOCK_STREAM;
    //        struct addrinfo* peer_address;
    //
    //        if (getaddrinfo(addr.data(), port.data(), &hints, &peer_address))
    //        {
    // log_agent->error(std::format("getaddrinfo() failed on call to connect() for TCP connection to {}:{}", addr, port));
    //        }
    //
    //        // printf("Remote address is: ");
    //        // char address_buffer[100];
    //        // char service_buffer[100];
    //        // getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
    //        //     address_buffer, sizeof(address_buffer),
    //        //     service_buffer, sizeof(service_buffer),
    //        //     NI_NUMERICHOST);
    //        // printf("%s %s\n", address_buffer, service_buffer);
    //
    //        if (connect(tcp_socket_descriptor, peer_address->ai_addr, peer_address->ai_addrlen) == -1)
    //        {
    // log_agent->error(std::format("call to connect() failed for TCP connection to {}:{}", addr, port));
    //        }
    //        else
    //        {
    // log_agent->info(std::format("TCP connection established to {}:{}", addr, port));
    //        }
    //
    //        freeaddrinfo(peer_address);
    //
    //        return 0;
    //    }

    int TcpSocket::tcp_bind()
    {
        // TODO: return true or false depending on failing to successding
        if (bind_address)
        {
            return bind(
                tcp_socket_descriptor,
                bind_address->ai_addr,
                bind_address->ai_addrlen
            );
        }

        return -1;
    }

    int TcpSocket::tcp_listen()
    {
        return listen(tcp_socket_descriptor, MAX_CONNECTIONS);
    }

    std::unique_ptr<Socket> TcpSocket::tcp_accept()
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
            return {};
        }
        else
        {
            // log new connection (use page 93 of c network programming)
            std::string host_name;
            tcp_get_host_name(reinterpret_cast<sockaddr&>(client_address), host_name);
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

    int TcpSocket::tcp_read(std::span<uint8_t> buffer, int flags = 0)
    {
        return recv(tcp_socket_descriptor, buffer.data(), buffer.size(), flags);
    }

    int TcpSocket::tcp_send(std::span<uint8_t> buffer)
    {
        return send(tcp_socket_descriptor, buffer.data(), buffer.size(), 0);
    }

    int TcpSocket::tcp_send(std::string_view buffer)
    {
        return send(tcp_socket_descriptor, buffer.data(), buffer.size(), 0);
    }

    void TcpSocket::tcp_close() { close(tcp_socket_descriptor); }

    const int TcpSocket::get_tcp_socket_descriptor() const { return tcp_socket_descriptor; }

    const std::string& TcpSocket::get_port() const { return port; }
}
