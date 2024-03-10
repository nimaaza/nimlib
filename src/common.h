#pragma once

enum ConnectionState { STARTING, READING, HANDLING, WRITING, PENDING, DONE, ERROR };
enum ParseResult { WRITE_AND_DIE, WRITE_AND_WAIT, INCOMPLETE };

const int BUFFER_SIZE = 10240;
const int MAX_RESET_COUNT = 10;

// Metric name constants
const std::string TIME_TO_RESPONSE {"resp_tm"};
