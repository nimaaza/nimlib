#pragma once

#include <vector>
#include <string>
#include <string_view>

void split(const std::string_view s, const std::string& delimiter, std::vector<std::string_view>& splits);
bool read_chunk(std::istream& source, std::string& buffer, long source_size, long chunk_size, long chunk_number);
