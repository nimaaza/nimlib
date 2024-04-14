#include <gtest/gtest.h>

#include <memory>
#include "iostream"

#include "../src/common/types.h"
#include "../src/connection.h"
#include "support/tcp_socket.mock.h"

using nimlib::Server::Connection;
using nimlib::Server::Sockets::MockTcpSocket;
using nimlib::Server::Types::ProtocolInterface;
using nimlib::Server::Types::ConnectionInterface;
using nimlib::Server::Types::StreamsProviderInterface;
using nimlib::Server::Constants::ParseResult;
using nimlib::Server::Constants::ConnectionState;

struct MockProtocolParser : public ProtocolInterface
{
    MockProtocolParser(
        StreamsProviderInterface& streams,
        int tries = 1,
        ParseResult parse_result = ParseResult::WRITE_AND_DIE
    )
        :
        in{ streams.get_input_stream() },
        out{ streams.get_output_stream() },
        total_tries{ tries },
        parse_result{ parse_result }
    {};
    ~MockProtocolParser() = default;

    void notify(ConnectionInterface& connection, StreamsProviderInterface& streams) override
    {
        tries_so_far++;

        // Pretend we have used the input stream.
        char c;
        while (in >> c)
        {
            internal_input += c;
        }

        connection.notify(*this);
    }

    void notify(ProtocolInterface& protocol, ConnectionInterface& connection, StreamsProviderInterface& streams) override {}

    bool wants_more_bytes() override { return tries_so_far != total_tries; }

    bool wants_to_write() override { return tries_so_far == total_tries; }

    bool wants_to_live() override { return parse_result == ParseResult::WRITE_AND_WAIT; }

    std::stringstream& in;
    std::stringstream& out;
    ParseResult parse_result;
    std::string internal_input{};
    int total_tries;
    int tries_so_far{ 0 };

    // FRIEND_TEST(ABC, XYZ);
};

TEST(ConnectionTests, Read_WithEnoughBuffer_SingleRead)
{
    /*
    Test case to test behaviour of a Connection object with a large enough buffer.
    The expected behaviour is to read the whole data available from the socket,
    push the data on its input stream.
    The mock protocol object mocks the behaviour of a successful parsing
    of input data. The protocol must instruct the connection to write the output
    stream to socket and close the connection. This happens when the connection
    ends up in the DONE state.
    */

    // Socket has 470 bytes available to be read.
    auto socket = std::make_unique<MockTcpSocket>(1, 470, 1024);
    MockTcpSocket* pointer_to_socket = socket.get(); // Steal the pointer to the socket object for later use.
    Connection connection{ std::move(socket), 1, 1024 };
    auto protocol = std::make_shared<MockProtocolParser>(connection, 1, ParseResult::WRITE_AND_DIE);
    connection.set_protocol(protocol);

    connection.read();

    auto [state_1, _] = connection.get_state();
    EXPECT_EQ(connection.get_input_stream().str().size(), 470);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);
    EXPECT_EQ(state_1, ConnectionState::WRITING);

    // When connection gets eventually picked for writing to socket...
    connection.write();

    auto [state_2, _2] = connection.get_state();
    EXPECT_EQ(state_2, ConnectionState::DONE);
}

TEST(ConnectionTests, Read_WithEnoughBuffer_MultipleReads)
{
    /*
    Test case to test behaviour of a Connection object when the protocol
    requests multiple reads. The protocol object will receive the necessary
    bytes to process over several reads and once all required bytes are
    provided, asks the connection to write the response and close. This can
    happen when the client doesn't send all the required bytes together.
    */

    auto socket = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    MockTcpSocket* pointer_to_socket = socket.get();
    socket->byte_counts_to_read.push_back(129);
    socket->byte_counts_to_read.push_back(131);
    socket->byte_counts_to_read.push_back(257);
    Connection connection{ std::move(socket), 1, 1024 };
    auto protocol = std::make_shared<MockProtocolParser>(connection, 3, ParseResult::WRITE_AND_DIE);
    connection.set_protocol(protocol);

    connection.read();

    auto state_1 = connection.get_state().first;
    EXPECT_EQ(state_1, ConnectionState::READING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 129);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);

    connection.read();

    auto state_2 = connection.get_state().first;
    EXPECT_EQ(state_2, ConnectionState::READING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 129 + 131);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);

    connection.read();

    auto state_3 = connection.get_state().first;
    EXPECT_EQ(state_3, ConnectionState::WRITING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 129 + 131 + 257);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);

    connection.write();

    auto state_4 = connection.get_state().first;
    EXPECT_EQ(state_4, ConnectionState::DONE);
}

TEST(ConnectionTests, Read_WithExactlyEnoughBuffer)
{
    /*
    Small test case for the unusual case when the Connection object buffer
    has exactly the size of the data available in the socket. The state of
    the Connection object will be ConnectionState::WRITING.
    */

    auto socket = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    socket->byte_counts_to_read = { 10 };
    MockTcpSocket* pointer_to_socket = socket.get();
    Connection connection{ std::move(socket), 1, 10 };
    auto protocol = std::make_shared<MockProtocolParser>(connection, 1, ParseResult::WRITE_AND_DIE);
    connection.set_protocol(protocol);

    connection.read();

    auto state = connection.get_state().first;
    EXPECT_EQ(state, ConnectionState::WRITING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 10);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);
}

TEST(ConnectionTests, Read_WithSmallBuffer)
{
    /*
    When the Connection object has a small buffer, ie., all the data the
    application requires cannot be read by the object at once, the Protocol
    object is expected to report incomplete data (ParseResult::INCOMPLETE)
    and the Connection object will try to read more data from the socket.

    Let's say the Protocol object needs 470 bytes to process but the Connection
    object has a buffer of only 167 bytes (all completely arbitrary). Since
    470 / 167 ~= 2.8, the Protocol needs to ask the Connection to read the
    socket 3 times to get all the required 470 bytes.
    */

    auto socket = std::make_unique<MockTcpSocket>(1, 470, 1024);
    MockTcpSocket* pointer_to_socket = socket.get();
    Connection connection{ std::move(socket), 1, 167 };
    auto protocol = std::make_shared<MockProtocolParser>(connection, 3, ParseResult::WRITE_AND_DIE);
    connection.set_protocol(protocol);

    connection.read();

    auto state_1 = connection.get_state().first;
    EXPECT_EQ(state_1, ConnectionState::READING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 167);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);

    connection.read();

    auto state_2 = connection.get_state().first;
    EXPECT_EQ(state_2, ConnectionState::READING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 167 + 167);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);

    connection.read();

    auto state_3 = connection.get_state().first;
    EXPECT_EQ(state_3, ConnectionState::WRITING);
    EXPECT_EQ(connection.get_input_stream().str().size(), 470);
    EXPECT_EQ(pointer_to_socket->read_result.str(), protocol->internal_input);

    connection.write();

    auto state_4 = connection.get_state().first;
    EXPECT_EQ(state_4, ConnectionState::DONE);
}

TEST(ConnectionTests, Read_ConnectionStateWhenSocketNotReady)
{
    // Socket not in ready state is mocked by setting 0 as the max number of
    // bytes that can be read off the socket.
    auto s = std::make_unique<MockTcpSocket>(1, 0, 0);
    Connection c{ std::move(s), 1 };

    c.read();

    auto [state, _] = c.get_state();
    EXPECT_EQ(state, ConnectionState::CON_ERROR);
}

TEST(ConnectionTests, Read_ConnectionStateWhenConnectionInErrorState)
{
    auto s = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    auto pointer_to_socket = s.get();
    Connection c{ std::move(s), 1 };
    c.halt(); // This forces connection in error state.

    auto socket_read_result_before = pointer_to_socket->read_result.str();
    auto read_result = c.read();
    auto socket_read_result_after = pointer_to_socket->read_result.str();
    EXPECT_EQ(read_result, ConnectionState::CON_ERROR);
    EXPECT_EQ(socket_read_result_before, socket_read_result_after);
}

TEST(ConnectionTests, Write_NoWriteWhenInError)
{
    auto s = std::make_unique<MockTcpSocket>(1);
    auto pointer_to_socket = s.get();
    Connection connection { std::move(s), 1 };
    connection.halt(); // This forces connection in error state.

    connection.write();

    EXPECT_EQ(pointer_to_socket->write_result.str(), "");
    EXPECT_EQ(pointer_to_socket->total_socket_write_count, 0);
}

TEST(ConnectionTests, Write_ConnectionKeptAlive)
{
    auto s = std::make_unique<MockTcpSocket>(1);
    auto pointer_to_socket = s.get();
    Connection connection{ std::move(s), 1 };
    auto protocol = std::make_shared<MockProtocolParser>(connection, 1, ParseResult::WRITE_AND_WAIT);
    connection.set_protocol(protocol);

    connection.write();

    EXPECT_EQ(connection.get_state().first, ConnectionState::PENDING);
}

TEST(ConnectionTests, ConnectionState_WhenJustCreated)
{
    auto s = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    Connection c{ std::move(s), 1 };

    auto [state, _] = c.get_state();
    EXPECT_EQ(state, ConnectionState::STARTING);
}

TEST(ConnectionTests, ConnectionState_WhenCreatedWithNullSocket)
{
    Connection c{ nullptr, 0 };

    auto [state, elapsed] = c.get_state();
    EXPECT_EQ(state, ConnectionState::CON_ERROR);
}

TEST(ConnectionTests, ConnectionState_WhenHalted)
{
    auto s = std::make_unique<MockTcpSocket>(1, 1024, 1024);
    Connection c{ std::move(s), 1 };

    c.halt();

    auto [state, elapsed] = c.get_state();
    EXPECT_EQ(state, ConnectionState::CON_ERROR);
}
