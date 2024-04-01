#pragma once

#include <string>
#include <unordered_map>

namespace nimlib::Server::Constants
{
    enum ConnectionState { STARTING, READING, HANDLING, WRITING, PENDING, DONE, CON_ERROR };

    enum ParseResult { WRITE_AND_DIE, WRITE_AND_WAIT, INCOMPLETE };

    enum LogLevel { INFO, DEBUG, WARN, ERROR, CRITICAL };

    const std::unordered_map<LogLevel, std::string_view> level_to_string_translation = {
            {LogLevel::INFO,     "info"},
            {LogLevel::DEBUG,    "debug"},
            {LogLevel::WARN,     "warn"},
            {LogLevel::ERROR,    "error"},
            {LogLevel::CRITICAL, "critical"}
    };

    const int BUFFER_SIZE = 10240;
    const int MAX_RESET_COUNT = 10;

    // Metric name constants
    const std::string TIME_TO_RESPONSE{ "resp_tm" };
};
