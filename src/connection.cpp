#include "connection.h"
#include "tls_layer.h"
#include "protocol.h"

#include <memory>
#include <utility>
#include <string_view>
#include <vector>

namespace nimlib::Server
{
    Connection::Connection(std::unique_ptr<TcpSocketInterface> s, connection_id id, size_t buffer_size)
        : id{ id },
        buffer_size{ buffer_size },
        connection_state{ ConnectionState::STARTING, ConnectionState::CON_ERROR, nimlib::Server::Constants::MAX_RESET_COUNT },
        socket{ std::move(s) },
        request_stream{},
        response_stream{},
        parse_result{ ParseResult::INCOMPLETE }
        //        response_timer{ nimlib::Server::Constants::TIME_TO_RESPONSE }
    {
        //        response_timer.start();

        if (!this->socket)
        {
            connection_state.set_state(ConnectionState::CON_ERROR);
        }

        protocol = std::make_shared<nimlib::Server::Protocols::Protocol>();
        // protocol = std::make_shared<nimlib::Server::Protocols::TlsLayer>(request_stream, response_stream, http_protocol);
    }

    Connection::~Connection()
    {
        //        response_timer.end();
    }

    ConnectionState Connection::read()
    {
        if (connection_state.get_state().first == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        connection_state.set_state(ConnectionState::READING);

        std::vector<uint8_t> buff(buffer_size, 0);
        int bytes_count = socket->tcp_read(buff, MSG_DONTWAIT);

        if (bytes_count > 0)
        {
            for (int i = 0; i < bytes_count && i < buffer_size; i++)
            {
                request_stream << buff[i];
            }

            connection_state.set_state(ConnectionState::HANDLING);
            protocol->parse(*this);
            return connection_state.get_state().first;
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
            return connection_state.set_state(ConnectionState::CON_ERROR);
        }
    }

    ConnectionState Connection::write()
    {
        if (connection_state.get_state().first == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        // TODO: trim response string?
        std::string response_str{ response_stream.str() };
        std::string_view response_view{ response_str };
        size_t bytes_to_send{ response_str.size() };
        size_t total_bytes_sent{};

        while (bytes_to_send > 0)
        {
            int sent = socket->tcp_send(response_view.substr(total_bytes_sent, buffer_size));

            if (sent < 0) break;

            bytes_to_send -= sent;
            total_bytes_sent += sent;
        }

        if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_DIE)
        {
            return connection_state.set_state(ConnectionState::DONE);
        }
        else if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_WAIT)
        {
            // TODO: reset connection variables or use state callbacks for clean up
            request_stream = std::stringstream();
            response_stream = std::stringstream();
            return connection_state.set_state(ConnectionState::PENDING);
        }
        else if (bytes_to_send > 0)
        {
            // TODO: erase might be faster
            response_stream.str(response_str.substr(total_bytes_sent));
            return connection_state.reset_state();
        }
        else
        {
            //TODO: crash? bytes_to_send cannot be negative
            return connection_state.set_state(ConnectionState::CON_ERROR);
        }
    }

    void Connection::halt()
    {
        connection_state.set_state(ConnectionState::CON_ERROR);
    }

    void Connection::set_parse_state(ParseResult pr)
    {
        parse_result = pr;

        // No assumption is made about how the streams will be used by the
        // application layer. The clear() method is being called in case
        // the application puts the streams in an error state.
        request_stream.clear();
        response_stream.clear();

        if (parse_result == ParseResult::WRITE_AND_DIE || parse_result == ParseResult::WRITE_AND_WAIT)
        {
            connection_state.set_state(ConnectionState::WRITING);
        }
        else if (parse_result == ParseResult::INCOMPLETE)
        {
            connection_state.set_state(ConnectionState::READING);
            connection_state.reset_state();
        }
        else
        {
            connection_state.set_state(ConnectionState::CON_ERROR);
        }
    }

    // Connection& Connection::operator<<(uint8_t c) {}

    // Connection& Connection::operator<<(std::string& s) {}

    std::stringstream& Connection::get_input_stream() { return request_stream; }

    std::stringstream& Connection::get_output_stream() { return response_stream; }

    void Connection::set_protocol(std::shared_ptr<ProtocolInterface> p) { protocol = p; }

    std::pair<ConnectionState, long> Connection::get_state() const { return connection_state.get_state(); }

    const int Connection::get_id() const { return id; }
}
