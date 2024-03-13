#include <iostream>

#include "version.h"
#include "polling_server.h"

int main()
{
    nimlib::Server::PollingServer ps {"8080"};
    ps.run();
}
