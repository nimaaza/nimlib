#include <sys/socket.h>
#include <utility>
#include <string_view>
#include <vector>
#include <ranges>

#include "connection.h"

Connection::Connection(std::unique_ptr<TcpSocket> socket)
    : protocol{},
    socket{ std::move(socket) },
    request_stream{},
    response_stream{},
    state{ ConnectionState::STARTING },
    parse_result{ ParseResult::STILL_NEED_MORE },
    last_state_change{},
    reset_count{}
{}

Connection::~Connection() {}

void Connection::read()
{
    set_state(ConnectionState::READING);

    std::array<char, BUFFER_SIZE> buff{};
    int bytes_count = socket->tcp_read(buff, MSG_DONTWAIT);

    if (bytes_count > 0)
    {
        for (int i = 0; i < bytes_count && i < BUFFER_SIZE; i++)
        {
            request_stream << buff[i];
        }

        handle_incoming_data();
    }
    else if (bytes_count == 0)
    {
        // TODO: still no data to read, what to do here?
        // This may never happen because when poll returns
        // there must be something to read.
        reset_state();
    }
    else
    {
        set_state(ConnectionState::ERROR);
    }
}

void Connection::handle_incoming_data()
{
    set_state(ConnectionState::HANDLING);
    parse_result = protocol.parse(request_stream, response_stream);

    if (parse_result == ParseResult::WRITE_AND_DIE || parse_result == ParseResult::WRITE_AND_WAIT)
    {
        set_state(ConnectionState::WRITING);
    }
    else if (parse_result == ParseResult::STILL_NEED_MORE)
    {
        set_state(ConnectionState::READING);
        reset_state();
    }
    else
    {
        set_state(ConnectionState::ERROR);
    }
}

void Connection::write()
{
    std::string response_str{ response_stream.str() };
    std::string_view response_view{ response_str };
    size_t bytes_to_send{ response_str.size() };
    size_t total_bytes_sent{};

    while (bytes_to_send > 0)
    {
        int sent = socket->tcp_send(response_view.substr(total_bytes_sent, BUFFER_SIZE));

        if (sent < 0) break;

        bytes_to_send -= sent;
        total_bytes_sent += sent;
    }

    if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_DIE)
    {
        set_state(ConnectionState::DONE);
    }
    else if (bytes_to_send == 0 && parse_result == ParseResult::WRITE_AND_WAIT)
    {
        set_state(ConnectionState::PENDING);
    }
    else if (bytes_to_send > 0)
    {
        // TODO: erase might be faster
        response_stream.str(response_str.substr(total_bytes_sent));
        reset_state();
    }
    else
    {
        //TODO: crash? bytes_to_send cannot be negative
        set_state(ConnectionState::ERROR);
    }
}

void Connection::halt()
{
    set_state(ConnectionState::ERROR);
}

void Connection::set_state(ConnectionState new_state)
{
    if (new_state == ConnectionState::PENDING)
    {
        // When the connection is to be reused, the state of the connection
        // object must be reset.
        request_stream.str("");
        response_stream.str("");
        reset_count = 0;
        parse_result = ParseResult::STILL_NEED_MORE;
    }

    if (new_state == state)
    {
        reset_state();
    }
    else
    {
        state = new_state;
    }

    last_state_change = std::chrono::high_resolution_clock::now();
}

void Connection::reset_state()
{
    reset_count++;

    if (reset_count >= MAX_RESET_COUNT)
    {
        set_state(ConnectionState::ERROR);
    }

    last_state_change = std::chrono::high_resolution_clock::now();
}

std::pair<ConnectionState, long> Connection::get_state() const
{
    auto now = std::chrono::high_resolution_clock::now();
    long elapsed = std::chrono::duration_cast<std::chrono::nanoseconds>(now - last_state_change).count();
    return { state, elapsed };
}

int Connection::get_tcp_socket_descriptor() const
{
    return socket->get_tcp_socket_descriptor();
}
