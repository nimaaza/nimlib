#include <gtest/gtest.h>
#include <memory>

#include "../src/tcp_socket.h"
#include "../src/socket.h"
#include "../src/connection.h"
#include "../src/common.h"

using nimlib::Server::Connection;
using nimlib::Server::Types::ProtocolInterface;
using nimlib::Server::Constants::ParseResult;
using nimlib::Server::Constants::ConnectionState;

struct MockProtocolParser : public ProtocolInterface
{
  ParseResult parse() override { return ParseResult::WRITE_AND_DIE; }
};

TEST(ConnectionState_WhenToldToRead, SocketNotReady)
{
  auto s = std::make_unique<TcpSocketAdapter>("8080");
  Connection c{ std::move(s), 1 };

  c.read();

  auto [state, _] = c.get_state();
  EXPECT_EQ(state, ConnectionState::CON_ERROR);
}

TEST(ConnectionState_WhenToldToRead, ConnectionInErrorState)
{
  auto s = std::make_unique<TcpSocketAdapter>("8080");
  Connection c{ std::move(s), 1 };

  c.halt(); // This forces connection in error state.

  auto read_result = c.read();
  EXPECT_EQ(read_result, ConnectionState::CON_ERROR);
}

TEST(ConnectionState_WhenCreated, ValidSocket)
{
  auto s = std::make_unique<TcpSocketAdapter>("8080");
  Connection c{ std::move(s), 1 };

  auto [state, elapsed] = c.get_state();
  EXPECT_EQ(state, ConnectionState::STARTING);
}

TEST(ConnectionState_WhenCreated, NullSocket)
{
  Connection c{ nullptr, 0 };

  auto [state, elapsed] = c.get_state();
  EXPECT_EQ(state, ConnectionState::CON_ERROR);
}

TEST(ConnectionState_WhenHalted, _)
{
  auto s = std::make_unique<TcpSocketAdapter>("8080");
  Connection c{ std::move(s), 1 };

  c.halt();

  auto [state, elapsed] = c.get_state();
  EXPECT_EQ(state, ConnectionState::CON_ERROR);
}
