#include <iostream>

#include "polling_server.h"
#include "decorators.h"

int main()
{
    using nimlib::Server::PollingServer;
    auto psl = nimlib::Server::Decorators::decorate(std::make_unique<PollingServer>("8080"));
    psl->run();
}
