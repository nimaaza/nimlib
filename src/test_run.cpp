#include <iostream>

#include "version.h"
#include "polling_server.h"

int main()
{
    PollingServer ps {"8080"};
    ps.run();
}
