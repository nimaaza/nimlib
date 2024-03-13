#pragma once

#include "logger.h"
#include "agent.h"
#include "../common.h"

namespace nimlib::Logging::Factory
{
    std::shared_ptr<LoggerAgent> get_agent(const std::string& agent_name, LogLevel level = LogLevel::INFO);
};
