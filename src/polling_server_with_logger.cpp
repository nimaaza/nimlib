#include <format>

#include "polling_server_with_logger.h"
#include "version.h"

namespace nimlib::Server
{
    PollingServerWithLogger::PollingServerWithLogger(PollingServerInterface& polling_server)
        : polling_server{ polling_server },
        main_log_agent{ nimlib::Server::Logging::Factory::get_agent("main") }
    {}

    PollingServerWithLogger::~PollingServerWithLogger()
    {
        main_log_agent->log("server shutting down");
    }

    void PollingServerWithLogger::run()
    {
        main_log_agent->log(
            std::format("server version {}.{}.{} running",
                nimlib_VERSION_MAJOR,
                nimlib_VERSION_MINOR,
                nimlib_VERSION_PATCH)
        );
        polling_server.run();
    }
}
