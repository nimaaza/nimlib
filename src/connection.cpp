#include "connection.h"
#include "state_manager.h"
//#include "tls_layer.h"
#include "protocol.h"

#include <memory>
#include <utility>
#include <string_view>
#include <vector>

namespace nimlib::Server
{
    Connection::Connection(std::unique_ptr<Socket> s, connection_id id)
        : id{ id },
        sm{ ConnectionState::STARTING, ConnectionState::CON_ERROR, nimlib::Server::Constants::MAX_RESET_COUNT },
        socket{ std::move(s) },
        request_stream{},
        response_stream{},
        parse_result{ ParseResult::INCOMPLETE }
        //        response_timer{ nimlib::Server::Constants::TIME_TO_RESPONSE }
    {
        //        response_timer.start();

        if (!this->socket)
        {
            sm.set_state(ConnectionState::CON_ERROR);
        }

        protocol = std::make_shared<nimlib::Server::Protocols::Protocol>(request_stream, response_stream);
        // protocol = std::make_shared<nimlib::Server::Protocols::TlsLayer>(request_stream, response_stream, http_protocol);
    }

    Connection::~Connection()
    {
        //        response_timer.end();
    }

    ConnectionState Connection::read()
    {
        if (sm.get_state().first == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        sm.set_state(ConnectionState::READING);

        std::array<uint8_t, nimlib::Server::Constants::BUFFER_SIZE> buff{};
        int bytes_count = socket->read(buff);

        if (bytes_count > 0)
        {
            for (int i = 0; i < bytes_count && i < nimlib::Server::Constants::BUFFER_SIZE; i++)
            {
                request_stream << buff[i];
            }

            return handle_incoming_data();
        }
        else if (bytes_count == 0)
        {
            // TODO: still no data to read, what to do here?
            // This may never happen because when poll returns
            // there must be something to read.
            return sm.reset_state();
        }
        else
        {
            return sm.set_state(ConnectionState::CON_ERROR);
        }
    }

    ConnectionState Connection::handle_incoming_data()
    {
        if (sm.get_state().first == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        sm.set_state(ConnectionState::HANDLING);
        parse_result = protocol->parse();

        if (parse_result == ParseResult::WRITE_AND_DIE || parse_result == ParseResult::WRITE_AND_WAIT)
        {
            return sm.set_state(ConnectionState::WRITING);
        }
        else if (parse_result == ParseResult::INCOMPLETE)
        {
            sm.set_state(ConnectionState::READING);
            return sm.reset_state();
        }
        else
        {
            return sm.set_state(ConnectionState::CON_ERROR);
        }
    }

    ConnectionState Connection::write()
    {
        if (sm.get_state().first == ConnectionState::CON_ERROR) return ConnectionState::CON_ERROR;

        // TODO: trim response string?
        std::string response_str{ response_stream.str() };
        std::string_view response_view{ response_str };
        size_t bytes_to_send{ response_str.size() };
        size_t total_bytes_sent{};

        while (bytes_to_send > 0)
        {
            int sent = socket->send(response_view.substr(total_bytes_sent, nimlib::Server::Constants::BUFFER_SIZE));

            if (sent < 0) break;

            bytes_to_send -= sent;
            total_bytes_sent += sent;
        }

        if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_DIE)
        {
            return sm.set_state(ConnectionState::DONE);
        }
        else if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_WAIT)
        {
            // TODO: reset connection variables or use state callbacks for clean up
            request_stream = std::stringstream();
            response_stream = std::stringstream();
            return sm.set_state(ConnectionState::PENDING);
        }
        else if (bytes_to_send > 0)
        {
            // TODO: erase might be faster
            response_stream.str(response_str.substr(total_bytes_sent));
            return sm.reset_state();
        }
        else
        {
            //TODO: crash? bytes_to_send cannot be negative
            return sm.set_state(ConnectionState::CON_ERROR);
        }
    }

    void Connection::halt()
    {
        sm.set_state(ConnectionState::CON_ERROR);
    }

    void Connection::set_protocol(std::shared_ptr<ProtocolInterface> p)
    {
        protocol = p;
    }


    std::pair<ConnectionState, long> Connection::get_state() const
    {
        return sm.get_state();
    }

    const int Connection::get_id() const
    {
        return id;
    }
}
