#include <sys/socket.h>
#include <memory>
#include <utility>
#include <string_view>
#include <vector>
#include <ranges>

#include "connection.h"
#include "state_manager.h"

Connection::Connection(std::unique_ptr<Socket> s, connection_id id)
    : id{ id },
    sm{ ConnectionState::STARTING, ConnectionState::ERROR, MAX_RESET_COUNT },
    socket{ std::move(s) },
    request_stream{},
    response_stream{},
    parse_result{ ParseResult::STILL_NEED_MORE }
{
    if (!this->socket)
    {
        sm.set_state(ConnectionState::ERROR);
    }

    protocol = std::make_shared<Protocol>();
}

Connection::~Connection() {}

ConnectionErrorCode Connection::read()
{
    if (sm.get_state().first == ConnectionState::ERROR) return ConnectionErrorCode::WONT_CONTINUE;

    sm.set_state(ConnectionState::READING);

    std::array<char, BUFFER_SIZE> buff{};
    int bytes_count = socket->read(buff);

    if (bytes_count > 0)
    {
        for (int i = 0; i < bytes_count && i < BUFFER_SIZE; i++)
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
        sm.reset_state();
        return ConnectionErrorCode::RESET;
    }
    else
    {
        sm.set_state(ConnectionState::ERROR);
        return ConnectionErrorCode::NOT_READY;
    }
}

ConnectionErrorCode Connection::handle_incoming_data()
{
    sm.set_state(ConnectionState::HANDLING);
    parse_result = protocol->parse(request_stream, response_stream);

    if (parse_result == ParseResult::WRITE_AND_DIE || parse_result == ParseResult::WRITE_AND_WAIT)
    {
        sm.set_state(ConnectionState::WRITING);
        return ConnectionErrorCode::OK;
    }
    else if (parse_result == ParseResult::STILL_NEED_MORE)
    {
        sm.set_state(ConnectionState::READING);
        sm.reset_state();
        return ConnectionErrorCode::RESET;
    }
    else
    {
        sm.set_state(ConnectionState::ERROR);
        return ConnectionErrorCode::WONT_CONTINUE;
    }
}

ConnectionErrorCode Connection::write()
{
    std::string response_str{ response_stream.str() };
    std::string_view response_view{ response_str };
    size_t bytes_to_send{ response_str.size() };
    size_t total_bytes_sent{};

    while (bytes_to_send > 0)
    {
        int sent = socket->send(response_view.substr(total_bytes_sent, BUFFER_SIZE));

        if (sent < 0) break;

        bytes_to_send -= sent;
        total_bytes_sent += sent;
    }

    if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_DIE)
    {
        sm.set_state(ConnectionState::DONE);
    }
    else if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_WAIT)
    {
        sm.set_state(ConnectionState::PENDING);
    }
    else if (bytes_to_send > 0)
    {
        // TODO: erase might be faster
        response_stream.str(response_str.substr(total_bytes_sent));
        sm.reset_state();
    }
    else
    {
        //TODO: crash? bytes_to_send cannot be negative
        sm.set_state(ConnectionState::ERROR);
    }

    return ConnectionErrorCode::OK; // TODO: correctly send code
}

void Connection::halt()
{
    sm.set_state(ConnectionState::ERROR);
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
