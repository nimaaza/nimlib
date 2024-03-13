#include "agent.h"

#include <chrono>
#include <iostream>
#include <format>
#include <unordered_map>

namespace nimlib::Server::Logging
{
    using LogLevel = nimlib::Server::Constants::LogLevel;

    Agent::Agent(const std::string& agent_name, LogLevel level)
        : master_logger{ nimlib::Server::Logging::Logger::get_instance() },
        level{ level },
        agent_name{ agent_name }
    {}

    Agent::~Agent() {}

    // bool Agent::set_format(std::string_view format) {}

    void Agent::set_level(LogLevel new_level) { level = new_level; }

    void Agent::log(std::string_view message) { print_message(LogLevel::INFO, message); }

    void Agent::log(LogLevel requested_level, std::string_view message)
    {
        if (requested_level >= level) print_message(requested_level, message);
    }

    void Agent::debug(std::string_view message) { log(LogLevel::DEBUG, message); }

    void Agent::info(std::string_view message) { log(LogLevel::INFO, message); }

    void Agent::warn(std::string_view message) { log(LogLevel::WARN, message); }

    void Agent::error(std::string_view message) { log(LogLevel::ERROR, message); }

    void Agent::critical(std::string_view message) { log(LogLevel::CRITICAL, message); }

    const std::string& Agent::get_name() { return agent_name; }

    void Agent::print_message(LogLevel requested_level, std::string_view message)
    {
        std::cout << format("[{0:%F} {0:%T}]: [{1:}] [{2:}] {3:}",
            std::chrono::system_clock::now(),
            nimlib::Server::Constants::level_to_string_translation.at(requested_level),
            agent_name,
            message) << std::endl;
    }
}
