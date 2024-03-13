#pragma once

#include "logger.h"
#include "agent.h"
#include "../common.h"

namespace nimlib::Server::Logging::Factory
{
    using LogLevel = nimlib::Server::Constants::LogLevel;

    std::shared_ptr<LoggerAgent> get_agent(const std::string& agent_name, LogLevel level = LogLevel::INFO);
};
