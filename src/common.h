#pragma once

#include <string>

enum ConnectionState { STARTING, READING, HANDLING, WRITING, PENDING, DONE, CON_ERROR };
enum ParseResult { WRITE_AND_DIE, WRITE_AND_WAIT, INCOMPLETE };
enum LogLevel { INFO, DEBUG, WARN, ERROR, CRITICAL };

const int BUFFER_SIZE = 10240;
const int MAX_RESET_COUNT = 10;

// Metric name constants
const std::string TIME_TO_RESPONSE{ "resp_tm" };

