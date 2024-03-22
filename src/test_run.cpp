#include <iostream>

#include "polling_server.h"
#include "decorators.h"

int main()
{
    nimlib::Server::PollingServer ps {"8080"};
	nimlib::Server::PollingServerWithLogger psl { ps};
    psl.run();
}
