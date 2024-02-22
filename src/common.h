#pragma once

enum ConnectionState { STARTING, READING, HANDLING, WRITING, PENDING, DONE, ERROR };
enum ParseResult { WRITE_AND_DIE, WRITE_AND_WAIT, STILL_NEED_MORE };
enum ConnectionErrorCode { OK, NOT_READY, RESET, WONT_CONTINUE };

const int BUFFER_SIZE = 10240;
const int MAX_RESET_COUNT = 10;