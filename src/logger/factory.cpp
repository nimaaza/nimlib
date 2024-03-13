#include <memory>

#include "factory.h"

namespace nimlib::Logging::Factory
{
    std::shared_ptr<LoggerAgent> get_agent(const std::string& agent_name)
    {
        auto agent = Logger::get_instance().get_agent(agent_name);
        if (agent)
        {
            return agent;
        }
        else
        {
            auto new_agent = std::make_shared<Agent>(agent_name);
            bool agent_registered = Logger::get_instance().register_agent(new_agent);
            if (agent_registered)
            {
                return new_agent;
            }
            else
            {
                return {}; // TODO: through exception
            }
        }
    }
};
