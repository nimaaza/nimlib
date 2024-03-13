#include "agent.h"

#include <chrono>
#include <iostream>
#include <format>

namespace nimlib::Logging
{

    Agent::Agent(const std::string& agent_name, LogLevel level)
        : master_logger{ nimlib::Logging::Logger::get_instance() },
        level{ level },
        agent_name{ agent_name }
    {}

    Agent::~Agent() {}

    // bool Agent::set_format(std::string_view format) {}

    void Agent::set_level(LogLevel new_level) { level = new_level; }

    void Agent::log(std::string_view message) { print_message(message); }

    void Agent::log(LogLevel level, std::string_view message) {}

    void Agent::debug(std::string_view message) { print_message(message); }

    void Agent::info(std::string_view message) { print_message(message); }

    void Agent::warn(std::string_view message) { print_message(message); }

    void Agent::error(std::string_view message) { print_message(message); }

    void Agent::critical(std::string_view message) { print_message(message); }

    const std::string& Agent::get_name() { return agent_name; }

    void Agent::print_message(std::string_view message)
    {
        std::cout << format("[{0:%F} {0:%T}]: [{1:}] {2:}", std::chrono::system_clock::now(), agent_name, message) << std::endl;
    }
}
