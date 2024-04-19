#pragma once

#include <format>

#include "../polling_server.h"
#include "../logger/factory.h"
#include "version.h"

namespace nimlib::Server
{
    class PollingServerWithLogger : public PollingServerInterface
    {
    public:
        explicit PollingServerWithLogger(std::unique_ptr<PollingServerInterface> polling_server)
            : polling_server{ std::move(polling_server) },
            main_log_agent{ nimlib::Server::Logging::Factory::get_agent("main") }
        {};

        ~PollingServerWithLogger()
        {
            main_log_agent->log("server shutting down");
        }

        PollingServerWithLogger(const PollingServerWithLogger&) = delete;
        PollingServerWithLogger& operator=(const PollingServerWithLogger&) = delete;
        PollingServerWithLogger(PollingServerWithLogger&&) noexcept = delete;
        PollingServerWithLogger& operator=(PollingServerWithLogger&&) noexcept = delete;

        void run() override
        {
            main_log_agent->log(
                std::format("server version {}.{}.{} running",
                    nimlib_VERSION_MAJOR,
                    nimlib_VERSION_MINOR,
                    nimlib_VERSION_PATCH)
            );
            polling_server->run();
        }

    private:
        std::unique_ptr<PollingServerInterface> polling_server;
        std::shared_ptr<nimlib::Server::Logging::LoggerAgent> main_log_agent;
    };
};
