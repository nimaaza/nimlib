#include "tcp_connection.h"

#include <memory>
#include <cassert>
#include <utility>
#include <string_view>
#include <vector>

namespace nimlib::Server
{
    const std::unordered_map<ConnectionState, std::vector<ConnectionState>> TcpConnection::states_transition_map
    {
        {ConnectionState::READY_TO_READ, {ConnectionState::INACTIVE, ConnectionState::READING}},
        {ConnectionState::READING, {ConnectionState::INACTIVE, ConnectionState::HANDLING}},
        {ConnectionState::HANDLING, {ConnectionState::INACTIVE, ConnectionState::READY_TO_READ, ConnectionState::READY_TO_WRITE}},
        {ConnectionState::READY_TO_WRITE, {ConnectionState::INACTIVE, ConnectionState::WRITING}},
        {ConnectionState::WRITING, {ConnectionState::INACTIVE, ConnectionState::READY_TO_WRITE, ConnectionState::READY_TO_READ, ConnectionState::DONE}},
        {ConnectionState::DONE, {ConnectionState::INACTIVE}},
        {ConnectionState::INACTIVE, {ConnectionState::READY_TO_READ}}
    };

    const std::unordered_map<ConnectionState, int> TcpConnection::max_reset_counts
    {
        { ConnectionState::READY_TO_READ, 1000},
        {ConnectionState::READING, 1000},
        {ConnectionState::HANDLING, 1000},
        {ConnectionState::READY_TO_WRITE, 1000},
        {ConnectionState::WRITING, 1000},
    };

    const std::unordered_map<ConnectionState, long> TcpConnection::state_time_outs
    {
        {ConnectionState::READY_TO_READ, 10'000},
        {ConnectionState::READING, 10'000},
        {ConnectionState::HANDLING, 10'000},
        {ConnectionState::READY_TO_WRITE, 10'000},
        {ConnectionState::WRITING, 10'000},
        {ConnectionState::DONE, 10'000},
    };

    TcpConnection::TcpConnection(std::unique_ptr<Socket> s, connection_id id, size_t buffer_size)
        : id{ id },
        buffer_size{ buffer_size },
        socket{ std::move(s) },
        response_timer{ nimlib::Server::Constants::TIME_TO_RESPONSE }
    {
        if (!this->socket)
        {
            connection_state.set_state(ConnectionState::CONNECTION_ERROR);
        }
    }

    TcpConnection::TcpConnection(connection_id id, size_t buffer_size)
        : id{ id },
        buffer_size{ buffer_size },
        socket{ nullptr },
        handler{ nullptr },
        response_timer{ nimlib::Server::Constants::TIME_TO_RESPONSE }
    {
        connection_state.set_state(ConnectionState::INACTIVE);
    }

    void TcpConnection::accept_socket(std::unique_ptr<Socket> s)
    {
        response_timer.start();
        socket = std::move(s);
        connection_state.set_state(ConnectionState::READY_TO_READ);
    }

    void TcpConnection::notify(ServerDirective directive)
    {
        (directive == ServerDirective::READ_SOCKET) ? read() : write();
    }

    void TcpConnection::notify(Handler& notifying_handler)
    {
        // No assumption is made about how the streams will be used by the
        // application layer. The clear() method is being called in case
        // the application puts the streams in an error state.
        input_stream.clear();
        output_stream.clear();

        keep_alive = notifying_handler.wants_to_live();

        if (notifying_handler.wants_to_write())
        {
            connection_state.set_state(ConnectionState::READY_TO_WRITE);
        }
        else if (notifying_handler.wants_more_bytes())
        {
            connection_state.set_state(ConnectionState::READY_TO_READ);
        }
        else
        {
            // TODO: make sure one of the calls above return true. Otherwise,
            // there is a logic error. The handler must have decided whether
            // it wants to send a response or read more bytes.
            connection_state.set_state(ConnectionState::CONNECTION_ERROR);
        }
    }

    void TcpConnection::set_handler(std::shared_ptr<Handler> h) { handler = h; }

    void TcpConnection::halt()
    {
        auto state = connection_state.get_state();

        if (state == ConnectionState::CONNECTION_ERROR)
        {
            connection_state.clear();
            connection_state.set_state(ConnectionState::INACTIVE);
            response_timer.cancel();
        }
        else
        {
            connection_state.set_state(ConnectionState::INACTIVE);
            response_timer.end();
        }

        input_stream.str("");
        output_stream.str("");

        if (socket)
        {
            socket->tcp_close();
            socket.reset();
        }
    }

    ConnectionState TcpConnection::get_state() { return connection_state.get_state(); }

    const int TcpConnection::get_id() const { return id; }

    std::stringstream& TcpConnection::source() { return input_stream; }

    std::stringstream& TcpConnection::sink() { return output_stream; }

    ConnectionState TcpConnection::read()
    {
        if (!connection_state.can_transition_to(ConnectionState::READING))
        {
            return connection_state.set_state(ConnectionState::CONNECTION_ERROR);
        }

        connection_state.set_state(ConnectionState::READING);

        std::vector<uint8_t> buff(buffer_size, 0);
        int bytes_count = socket->tcp_read(buff, MSG_DONTWAIT);

        if (bytes_count > 0)
        {
            for (int i = 0; i < bytes_count && i < buffer_size; i++)
            {
                input_stream << buff[i];
            }

            // Handler must not be null.
            assert(handler);

            connection_state.set_state(ConnectionState::HANDLING);
            handler->notify(*this, *this);
            return connection_state.get_state();
        }
        else if (bytes_count == 0)
        {
            // TODO: still no data to read, what to do here?
            // This may never happen because when poll returns
            // there must be something to read.
            return connection_state.reset_state();
        }
        else
        {
            // TODO: read has returned a negative value.
            // This means we might need some error handling here.
            return connection_state.set_state(ConnectionState::CONNECTION_ERROR);
        }
    }

    ConnectionState TcpConnection::write()
    {
        if (!connection_state.can_transition_to(ConnectionState::WRITING))
        {
            return connection_state.set_state(ConnectionState::CONNECTION_ERROR);
        }

        connection_state.set_state(ConnectionState::WRITING);
        std::string response_str{ std::move(output_stream.str()) };
        std::string_view response{ response_str };
        size_t bytes_to_send = response_str.size();
        size_t bytes_sent = 0;

        while (bytes_sent < bytes_to_send)
        {
            auto sent = socket->tcp_send(response.substr(bytes_sent));

            if (sent < 0)
            {
                break;
            }
            else
            {
                bytes_sent += sent;
            }
        }

        if (bytes_sent == bytes_to_send)
        {
            if (keep_alive)
            {
                //  TODO: reset connection variables or use state callbacks for clean up
                input_stream.str("");
                output_stream.str("");
                return connection_state.set_state(ConnectionState::READY_TO_READ);
            }
            else
            {
                return connection_state.set_state(ConnectionState::DONE);
            }
        }
        else if (bytes_sent < bytes_to_send)
        {
            std::string remainder{ response.substr(bytes_sent) };
            output_stream.str(remainder);
            return connection_state.set_state(ConnectionState::READY_TO_WRITE);
        }
        else
        {
            // There's a serious logic issue.
            assert(false);
        }
    }
}
