#include <gtest/gtest.h>

#include <memory>

#include "../src/common/types.h"
#include "../src/tcp_connection.h"
#include "support/tcp_socket.mock.h"

using nimlib::Server::TcpConnection;
using nimlib::Server::Sockets::MockTcpSocket;
using nimlib::Server::Types::Handler;
using nimlib::Server::Types::Connection;
using nimlib::Server::Types::StreamsProvider;
using nimlib::Server::Constants::HandlerState;
using nimlib::Server::Constants::ServerDirective;
using nimlib::Server::Constants::ConnectionState;

struct MockHandler : public Handler
{
    MockHandler(
        StreamsProvider& streams,
        int read_attempts = 1,
        HandlerState final_handler_state = HandlerState::WRITE_AND_DIE,
        std::string output_result = ""
    )
        :
        in{ streams.source() },
        out{ streams.sink() },
        total_read_attempts{ read_attempts },
        handler_state{ final_handler_state },
        output_result{ output_result }
    {};
    ~MockHandler() = default;

    void notify(Connection& connection, StreamsProvider& streams) override
    {
        read_attempts_so_far++;

        // Pretend we have used the input stream.
        char c;
        while (in >> c)
        {
            internal_input += c;
            if (output_result.empty()) out << c;
        }

        out << output_result;

        connection.notify(*this);
    }

    void notify(Handler& handler, Connection& connection, StreamsProvider& streams) override {}

    bool wants_more_bytes() override { return read_attempts_so_far != total_read_attempts; }

    bool wants_to_write() override { return read_attempts_so_far == total_read_attempts; }

    bool wants_to_live() override { return handler_state == HandlerState::WRITE_AND_WAIT; }

    std::stringstream& in;
    std::stringstream& out;
    HandlerState handler_state;
    std::string output_result;
    std::string internal_input{};
    int total_read_attempts;
    int read_attempts_so_far{ 0 };
};

TEST(ConnectionTests, Read_WithEnoughBuffer_SingleRead)
{
    /*
    Test case to test behaviour of a TcpConnection object with a large enough buffer.
    The expected behaviour is to read the whole data available from the socket,
    push the data on its input stream.
    The mock handler object mocks the behaviour of a successful parsing
    of input data. The handler must instruct the connection to write the output
    stream to socket and close the connection. This happens when the connection
    ends up in the INACTIVE state.
    */

    // Socket has 470 bytes available to be read.
    auto socket = std::make_unique<MockTcpSocket>(1, 470, 1024);
    MockTcpSocket* pointer_to_socket = socket.get(); // Steal the pointer to the socket object for later use.
    TcpConnection connection{ std::move(socket), 1, 1024 };
    auto handler = std::make_shared<MockHandler>(connection, 1, HandlerState::WRITE_AND_DIE);
    connection.set_handler(handler);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_1 = connection.get_state();
    EXPECT_EQ(connection.source().str().size(), 470);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);
    EXPECT_EQ(state_1, ConnectionState::READY_TO_WRITE);

    // When connection gets eventually picked for writing to socket...
    connection.notify(ServerDirective::WRITE_SOCKET);

    auto state_2 = connection.get_state();
    EXPECT_EQ(state_2, ConnectionState::DONE);
}

TEST(ConnectionTests, Read_WithEnoughBuffer_MultipleReads)
{
    /*
    Test case to test behaviour of a TcpConnection object when the handler
    requests multiple reads. The handler object will receive the necessary
    bytes to process over several reads and once all required bytes are
    provided, asks the connection to write the response and close. This can
    happen when the client doesn't send all the required bytes together.
    */

    auto socket = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    MockTcpSocket* pointer_to_socket = socket.get();
    socket->byte_counts_to_read.push_back(129);
    socket->byte_counts_to_read.push_back(131);
    socket->byte_counts_to_read.push_back(257);
    TcpConnection connection{ std::move(socket), 1, 1024 };
    auto handler = std::make_shared<MockHandler>(connection, 3, HandlerState::WRITE_AND_DIE);
    connection.set_handler(handler);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_1 = connection.get_state();
    EXPECT_EQ(state_1, ConnectionState::READY_TO_READ);
    EXPECT_EQ(connection.source().str().size(), 129);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_2 = connection.get_state();
    EXPECT_EQ(state_2, ConnectionState::READY_TO_READ);
    EXPECT_EQ(connection.source().str().size(), 129 + 131);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_3 = connection.get_state();
    EXPECT_EQ(state_3, ConnectionState::READY_TO_WRITE);
    EXPECT_EQ(connection.source().str().size(), 129 + 131 + 257);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);

    connection.notify(ServerDirective::WRITE_SOCKET);

    auto state_4 = connection.get_state();
    EXPECT_EQ(state_4, ConnectionState::DONE);
}

TEST(ConnectionTests, Read_WithExactlyEnoughBuffer)
{
    /*
    Small test case for the unusual case when the TcpConnection object buffer
    has exactly the size of the data available in the socket. The state of
    the TcpConnection object will be ConnectionState::READY_TO_WRITE.
    */

    auto socket = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    socket->byte_counts_to_read = { 10 };
    MockTcpSocket* pointer_to_socket = socket.get();
    TcpConnection connection{ std::move(socket), 1, 10 };
    auto handler = std::make_shared<MockHandler>(connection, 1, HandlerState::WRITE_AND_DIE);
    connection.set_handler(handler);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state = connection.get_state();
    EXPECT_EQ(state, ConnectionState::READY_TO_WRITE);
    EXPECT_EQ(connection.source().str().size(), 10);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);
}

TEST(ConnectionTests, Read_WithSmallBuffer)
{
    /*
    When the TcpConnection object has a small buffer, ie., all the data the
    application requires cannot be read by the object at once, the handler
    object is expected to report incomplete data (HandlerState::INCOMPLETE)
    and the TcpConnection object will try to read more data from the socket.

    Let's say the handler object needs 470 bytes to process but the TcpConnection
    object has a buffer of only 167 bytes (all completely arbitrary). Since
    470 / 167 ~= 2.8, the handler needs to ask the TcpConnection to read the
    socket 3 times to get all the required 470 bytes.
    */

    auto socket = std::make_unique<MockTcpSocket>(1, 470, 1024);
    MockTcpSocket* pointer_to_socket = socket.get();
    TcpConnection connection{ std::move(socket), 1, 167 };
    auto handler = std::make_shared<MockHandler>(connection, 3, HandlerState::WRITE_AND_DIE);
    connection.set_handler(handler);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_1 = connection.get_state();
    EXPECT_EQ(state_1, ConnectionState::READY_TO_READ);
    EXPECT_EQ(connection.source().str().size(), 167);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_2 = connection.get_state();
    EXPECT_EQ(state_2, ConnectionState::READY_TO_READ);
    EXPECT_EQ(connection.source().str().size(), 167 + 167);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);

    connection.notify(ServerDirective::READ_SOCKET);

    auto state_3 = connection.get_state();
    EXPECT_EQ(state_3, ConnectionState::READY_TO_WRITE);
    EXPECT_EQ(connection.source().str().size(), 470);
    EXPECT_EQ(pointer_to_socket->read_result.str(), handler->internal_input);

    connection.notify(ServerDirective::WRITE_SOCKET);

    auto state_4 = connection.get_state();
    EXPECT_EQ(state_4, ConnectionState::DONE);
}

TEST(ConnectionTests, Read_ConnectionStateWhenSocketNotReady)
{
    /*
    Socket not in ready state is mocked by setting 0 as the max number of
    bytes that can be read off the socket.
    */

    auto s = std::make_unique<MockTcpSocket>(1, 0, 0);
    TcpConnection c{ std::move(s), 1 };

    c.notify(ServerDirective::READ_SOCKET);

    auto state = c.get_state();
    EXPECT_EQ(state, ConnectionState::CONNECTION_ERROR);
}

// TEST(ConnectionTests, Read_ConnectionStateWhenConnectionInErrorState)
// {
    // TODO: in the current state, calling halt() won't simply
    // put the connection in an error state. This test must be
    // revised or it's irrelevant.
    // auto s = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    // auto pointer_to_socket = s.get();
    // TcpConnection c{ std::move(s), 1 };
    // c.halt(); // This forces connection in error state.

    // auto socket_read_result_before = pointer_to_socket->read_result.str();
    // c.notify(ServerDirective::READ_SOCKET);
    // auto read_result = c.get_state();
    // auto socket_read_result_after = pointer_to_socket->read_result.str();
    // EXPECT_EQ(read_result, ConnectionState::CONNECTION_ERROR);
    // EXPECT_EQ(socket_read_result_before, socket_read_result_after);
// }

// TEST(ConnectionTests, Write_NoWriteWhenInError)
// {
    // TODO: in the current state, calling halt() won't simply
    // put the connection in an error state. This test must be
    // revised or it's irrelevant.
    // auto s = std::make_unique<MockTcpSocket>(1);
    // auto pointer_to_socket = s.get();
    // TcpConnection connection{ std::move(s), 1 };
    // connection.halt(); // This forces connection in error state.

    // connection.notify(ServerDirective::WRITE_SOCKET);

    // EXPECT_EQ(pointer_to_socket->write_result.str(), "");
    // EXPECT_EQ(pointer_to_socket->total_socket_write_count, 0);
// }

TEST(ConnectionTests, Write_ConnectionKeepAlive)
{
    auto s = std::make_unique<MockTcpSocket>(1);
    auto pointer_to_socket = s.get();
    TcpConnection connection{ std::move(s), 1 };
    auto handler = std::make_shared<MockHandler>(
        connection,
        1,
        HandlerState::WRITE_AND_WAIT,
        "HTTP/1.1 404 Not Found"
    );
    connection.set_handler(handler);

    connection.notify(ServerDirective::READ_SOCKET);
    connection.notify(ServerDirective::WRITE_SOCKET);

    EXPECT_EQ(connection.get_state(), ConnectionState::READY_TO_READ);
    EXPECT_EQ(pointer_to_socket->total_socket_write_count, 22);
    EXPECT_EQ(pointer_to_socket->write_result.str(), "HTTP/1.1 404 Not Found");
}

TEST(ConnectionTests, Write_ConnectionClose)
{
    auto s = std::make_unique<MockTcpSocket>(1);
    auto pointer_to_socket = s.get();
    TcpConnection connection{ std::move(s), 1 };
    std::string expected_connection_output = "HTTP/1.1 404 Not Found\r\n\r\n";
    auto handler = std::make_shared<MockHandler>(
        connection,
        1,
        HandlerState::WRITE_AND_DIE,
        expected_connection_output
    );
    connection.set_handler(handler);

    connection.notify(ServerDirective::READ_SOCKET);
    connection.notify(ServerDirective::WRITE_SOCKET);

    EXPECT_EQ(connection.get_state(), ConnectionState::DONE);
    EXPECT_EQ(pointer_to_socket->total_socket_write_count, expected_connection_output.size());
    EXPECT_EQ(pointer_to_socket->write_result.str(), expected_connection_output);
}

TEST(ConnectionTests, Write_SeveralWrites)
{
    /*
     This test mimics the scenario when there are more bytes in the connection's
     output stream (a random number like 473 bytes) than can be written to the
     TCP socket. For example, the socket can transport only 127 bytes at each
     time when the send() function is called on the socket. Then the connection
     has to call send several times until it has sent all of its output stream. In
     this scenario, the socket still accepts subsequent writes without returning
     an error.
     */

    auto s = std::make_unique<MockTcpSocket>(1, 1024, 127);
    auto pointer_to_socket = s.get();
    TcpConnection connection{ std::move(s), 1, 473 };
    auto handler = std::make_shared<MockHandler>(connection);
    connection.set_handler(handler);
    auto& connection_streams = static_cast<StreamsProvider&>(connection);

    connection.notify(ServerDirective::READ_SOCKET);
    connection.notify(ServerDirective::WRITE_SOCKET);
    std::string expected_connection_output = connection_streams.sink().str();

    EXPECT_EQ(pointer_to_socket->total_socket_write_count, 473);
    EXPECT_EQ(pointer_to_socket->write_result.str(), expected_connection_output);
}

TEST(ConnectionTests, Write_WithError)
{
    /*
     This test mimics the scenario when writing to the sockets leads to
     an error. In this case the connection stops writing and will continue
     when notified to write again. The connection state is reset when this
     happens. If the reset happens more than a certain number of times, the
     connection will be put into an error state.
     The mock TCP socket has enough data to fill the 512 byte buffer of the
     connection object and the mocked handler object produces an output of
     the same size. But the mocked TCP socket can send only 4 * 64 bytes
     before it returns error. This has been done this way to mimic the case
     of the socket failing to complete the write operation.
     */

    auto s = std::make_unique<MockTcpSocket>(1, 1024, 64);
    auto pointer_to_socket = s.get();
    TcpConnection connection{ std::move(s), 1, 512 };
    auto handler = std::make_shared<MockHandler>(connection);
    connection.set_handler(handler);
    auto& connection_streams = static_cast<StreamsProvider&>(connection);
    connection.notify(ServerDirective::READ_SOCKET);
    std::string expected_connection_output{ connection_streams.sink().str() };

    connection.notify(ServerDirective::WRITE_SOCKET);
    int total_bytes_written = pointer_to_socket->total_socket_write_count;
    EXPECT_EQ(connection.get_state(), ConnectionState::READY_TO_WRITE);

    // Reset the socket write count so that the socket can be written to again.
    pointer_to_socket->total_socket_write_count = 0;
    connection.notify(ServerDirective::WRITE_SOCKET);
    total_bytes_written += pointer_to_socket->total_socket_write_count;

    EXPECT_EQ(total_bytes_written, 512);
    EXPECT_EQ(pointer_to_socket->write_result.str(), expected_connection_output);
}

TEST(ConnectionTests, ConnectionState_WhenJustCreated)
{
    auto s = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    TcpConnection c{ std::move(s), 1 };

    auto state = c.get_state();
    EXPECT_EQ(state, ConnectionState::READY_TO_READ);
}

TEST(ConnectionTests, ConnectionState_WhenCreatedWithNullSocket)
{
    TcpConnection c{ nullptr, 0 };

    auto state = c.get_state();
    EXPECT_EQ(state, ConnectionState::CONNECTION_ERROR);
}

TEST(ConnectionTests, ConnectionState_WhenHalted)
{
    auto s = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    TcpConnection c{ std::move(s), 1 };
    auto& connection_streams = static_cast<StreamsProvider&>(c);

    c.halt();

    auto state = c.get_state();
    EXPECT_EQ(state, ConnectionState::INACTIVE);
    EXPECT_EQ(connection_streams.source().str(), "");
    EXPECT_EQ(connection_streams.sink().str(), "");
}
