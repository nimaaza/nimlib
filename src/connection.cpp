#include "connection.h"

#include <memory>
#include <cassert>
#include <utility>
#include <string_view>
#include <vector>

namespace nimlib::Server
{
    Connection::Connection(std::unique_ptr<TcpSocketInterface> s, connection_id id, size_t buffer_size)
        : id{ id },
        buffer_size{ buffer_size },
        socket{ std::move(s) }
        // response_timer{ nimlib::Server::Constants::TIME_TO_RESPONSE }
    {
        // response_timer.start();

        if (!this->socket)
        {
            connection_state.set_state(ConnectionState::CON_ERROR);
        }
    }

    Connection::~Connection()
    {
        // response_timer.end();
    }

    ConnectionState Connection::read()
    {
        if (connection_state.get_state() == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        connection_state.set_state(ConnectionState::READING);

        std::vector<uint8_t> buff(buffer_size, 0);
        int bytes_count = socket->tcp_read(buff, MSG_DONTWAIT);

        if (bytes_count > 0)
        {
            for (int i = 0; i < bytes_count && i < buffer_size; i++)
            {
                input_stream << buff[i];
            }

            // Protocol must not be null.
            assert(protocol);

            connection_state.set_state(ConnectionState::HANDLING);
            protocol->notify(*this, *this);
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
            return connection_state.set_state(ConnectionState::CON_ERROR);
        }
    }

    ConnectionState Connection::write()
    {
        if (connection_state.get_state() == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        std::string response_str{ std::move(output_stream.str()) };
        std::string_view response{ response_str };
        long bytes_to_send = response_str.size();
        long bytes_sent = 0;

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
                return connection_state.set_state(ConnectionState::PENDING);
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
            return connection_state.reset_state();
        }
        else
        {
            // There's a serious logic issue.
            assert(false);
        }
    }

    void Connection::halt()
    {
        connection_state.set_state(ConnectionState::CON_ERROR);
        // socket->tcp_close(); TODO: it's better to close the socket carefully.
    }

    void Connection::notify(ProtocolInterface& protocol)
    {
        // No assumption is made about how the streams will be used by the
        // application layer. The clear() method is being called in case
        // the application puts the streams in an error state.
        input_stream.clear();
        output_stream.clear();

        keep_alive = protocol.wants_to_live();

        if (protocol.wants_to_write())
        {
            connection_state.set_state(ConnectionState::WRITING);
        }
        else if (protocol.wants_more_bytes())
        {
            connection_state.set_state(ConnectionState::READING);
        }
        else
        {
            connection_state.set_state(ConnectionState::CON_ERROR);
        }
    }

    void Connection::set_protocol(std::shared_ptr<ProtocolInterface> p) { protocol = p; }

    std::pair<ConnectionState, long> Connection::get_state() const { return connection_state.get_state_pair(); }

    const int Connection::get_id() const { return id; }

    std::stringstream& Connection::get_input_stream() { return input_stream; }

    std::stringstream& Connection::get_output_stream() { return output_stream; }
}
