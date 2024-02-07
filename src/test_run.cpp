#include "version.h"
#include "polling_server.h"

int main()
{
    std::cout << nimlib_VERSION_MAJOR << "." << nimlib_VERSION_MINOR << "." << nimlib_VERSION_PATCH << std::endl;

    PollingServer ps {"8080"};
    ps.run();
}
