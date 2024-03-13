#pragma once

#include "../common.h"
#include "logger.h"

#include <string_view>
#include <string>

namespace nimlib::Server::Logging
{
    using LogLevel = nimlib::Server::Constants::LogLevel;

    class Agent : public LoggerAgent
    {
    public:
        Agent(const std::string& agent_name, LogLevel level);
        ~Agent();

        Agent(const Agent&) = delete;
        Agent& operator=(const Agent&) = delete;
        Agent(Agent&&) noexcept = delete;
        Agent& operator=(Agent&&) noexcept = delete;

        void set_level(LogLevel new_level) override;
        void log(std::string_view message) override;
        void log(LogLevel requested_level, std::string_view message) override;

        void debug(std::string_view message) override;
        void info(std::string_view message) override;
        void warn(std::string_view message) override;
        void error(std::string_view message) override;
        void critical(std::string_view message) override;

        const std::string& get_name() override;

    private:
        void print_message(LogLevel requested_level, std::string_view message);

    private:
        nimlib::Server::Logging::Logger& master_logger;
        std::string agent_name;
        LogLevel level;
        std::string format_string;
    };
};
