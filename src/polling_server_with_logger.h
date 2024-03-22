#pragma once

#include "polling_server.h"
#include "logger/factory.h"

namespace nimlib::Server
{
    class PollingServerWithLogger : public PollingServerInterface
    {
    public:
        explicit PollingServerWithLogger(PollingServerInterface& polling_server);
        ~PollingServerWithLogger();

        PollingServerWithLogger(const PollingServerWithLogger&) = delete;
        PollingServerWithLogger& operator=(const PollingServerWithLogger&) = delete;
        PollingServerWithLogger(PollingServerWithLogger&&) noexcept = delete;
        PollingServerWithLogger& operator=(PollingServerWithLogger&&) noexcept = delete;

        void run() override;

    private:
        PollingServerInterface& polling_server;
        std::shared_ptr<nimlib::Server::Logging::LoggerAgent> main_log_agent;
    };
};
