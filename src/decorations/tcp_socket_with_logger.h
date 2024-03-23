#pragma once

#include <format>
#include <span>

#include "../types.h"
#include "../tcp_socket.h"
#include "../logger/factory.h"

namespace nimlib::Server::Sockets
{
    using nimlib::Server::Types::TcpSocketInterface;

	struct TcpSocketWithLogger : public TcpSocketInterface
    {
        TcpSocketWithLogger(std::unique_ptr<TcpSocketInterface> tcp_socket)
            : TcpSocketInterface{ tcp_socket->get_port(), tcp_socket->get_tcp_socket_descriptor() },
            tcp_socket{ std::move(tcp_socket) },
            log_agent{ nimlib::Server::Logging::Factory::get_agent("socket") }
        {};

        ~TcpSocketWithLogger()
        {
            log_agent->info(std::format("closing socket {} bound to port", tcp_socket_descriptor, port));
        };

        int tcp_bind()
        {
            auto bind_result = tcp_socket->tcp_bind();

            if (bind_result == 0)
            {
                log_agent->info(std::format("bound socket {} to port {}", tcp_socket_descriptor, port));
            }
            else
            {
                log_agent->error(std::format("failed to bind socket {} to port {}", tcp_socket_descriptor, port));
            }

            return bind_result;
        };

        int tcp_listen()
        {
            auto listen_result = tcp_socket->tcp_listen();

            if (listen_result < 0)
            {
                log_agent->error(std::format("socket {} failed to start listening on port {}", tcp_socket_descriptor, port));
            }
            else
            {
                log_agent->info(std::format("socket {} listening on port {}", tcp_socket_descriptor, port));
            }

            return listen_result;
        };

        std::unique_ptr<TcpSocketInterface> tcp_accept()
        {
            auto new_socket = tcp_socket->tcp_accept();

            if (new_socket)
            {
                log_agent->info(std::format("TCP socket {} accepted new connection", tcp_socket_descriptor));
            }
            else
            {
                log_agent->error(std::format("TCP socket {} failed to accept new connection", tcp_socket_descriptor));
            }

            return new_socket;
        };

        void tcp_get_host_name(const sockaddr& socket_address, std::string& host_name)
        {
            tcp_socket->tcp_get_host_name(socket_address, host_name);
        };

        int tcp_read(std::span<uint8_t> buffer, int flags)
        {
            int bytes_count = tcp_socket->tcp_read(buffer, flags);
            log_agent->info(std::format("read {} bytes from socket {}", bytes_count, tcp_socket_descriptor));
            return bytes_count;
        };

        int tcp_send(std::span<uint8_t> buffer)
        {
            int bytes_count = tcp_socket->tcp_send(buffer);
            log_agent->info(std::format("wrote {} bytes to socket {}", bytes_count, tcp_socket_descriptor));
            return bytes_count;
        };

        int tcp_send(std::string_view buffer)
        {
            int bytes_count = tcp_socket->tcp_send(buffer);
            log_agent->info(std::format("wrote {} bytes to socket {}", bytes_count, tcp_socket_descriptor));
            return bytes_count;
        };

        void tcp_close()
        {
            log_agent->info(std::format("TCP socket {} closing", tcp_socket_descriptor));
            tcp_socket->tcp_close();
        };

        const int get_tcp_socket_descriptor() const
        {
            return tcp_socket->get_tcp_socket_descriptor();
        };

        const std::string& get_port() const
        {
            return tcp_socket->get_port();
        };

    private:
        std::unique_ptr<TcpSocketInterface> tcp_socket;
        std::shared_ptr<nimlib::Server::Logging::LoggerAgent> log_agent;
    };
};
