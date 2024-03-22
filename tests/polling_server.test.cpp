#include <gtest/gtest.h>

#include "../src/polling_server.h"

using namespace nimlib::Server;

TEST(PollingServerTest, _)
{
    PollingServer ps{ "8080" };
}
