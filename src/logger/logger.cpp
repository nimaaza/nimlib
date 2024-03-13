#include "logger.h"

namespace nimlib::Logging
{
    Logger::Logger() {}

    Logger::~Logger() {}

    bool Logger::initialize() { return true; }

    bool Logger::halt() { return true; }

    bool Logger::register_agent(std::shared_ptr<LoggerAgent> agent)
    {
        auto agent_itr = agents.find(agent->get_name());
        if (agent_itr == agents.end())
        {
            agents[agent->get_name()] = agent;
            return true;
        }
        else
        {
            return false;
        }
    }

    std::shared_ptr<LoggerAgent> Logger::get_agent(const std::string& agent_name)
    {
        auto agent_itr = agents.find(agent_name);
        if (agent_itr != agents.end())
        {
            return agent_itr->second;
        }
        else
        {
            return {};
        }
    }

    Logger& Logger::get_instance()
    {
        static Logger logger{};
        return logger;
    }
}
