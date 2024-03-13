#pragma once

#include "logger.h"
#include "agent.h"

namespace nimlib::Logging::Factory
{
    std::shared_ptr<LoggerAgent> get_agent(const std::string& agent_name);
};
