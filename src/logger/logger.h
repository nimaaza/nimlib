#pragma once

#include "../common.h"

#include <string>
#include <string_view>
#include <unordered_map>
#include <memory>

namespace nimlib::Server::Logging
{
    struct LoggerAgent
    {
        virtual ~LoggerAgent() = default;

        virtual void set_level(LogLevel new_level) = 0;
        virtual void log(std::string_view message) = 0;
        virtual void log(LogLevel level, std::string_view message) = 0;

        virtual void debug(std::string_view message) = 0;
        virtual void info(std::string_view message) = 0;
        virtual void warn(std::string_view message) = 0;
        virtual void error(std::string_view message) = 0;
        virtual void critical(std::string_view message) = 0;

        virtual const std::string& get_name() = 0;
    };

    class Logger
    {
    private:
        Logger();

    public:
        ~Logger();

        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;
        Logger(Logger&&) noexcept = delete;
        Logger& operator=(Logger&&) noexcept = delete;

    public:
        bool initialize();
        bool halt();
        bool register_agent(std::shared_ptr<LoggerAgent> agent);
        std::shared_ptr<LoggerAgent> get_agent(const std::string& agent_name);

        static Logger& get_instance();

    private:
        std::unordered_map<std::string, std::shared_ptr<LoggerAgent>> agents{};
    };
}
