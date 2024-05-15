#include "helpers.h"

#include <istream>
#include <optional>

void split(
    const std::string_view s,
    const std::string& delimiter,
    std::vector<std::string_view>& splits
)
{
    if (delimiter.empty() || s.empty()) return;

    auto white_space = [](char c) -> bool { return std::isspace(c); };
    size_t begin = 0;
    size_t end = 0;

    while (end < s.size())
    {
        end = s.find(delimiter, begin);
        if (end == std::string::npos) end = s.size();

        // Excluding white space chars from beginning and end.
        long begin_no_ws = begin;
        long end_no_ws = end - 1;
        while (white_space(s[begin_no_ws])) begin_no_ws++;
        if (begin_no_ws < end_no_ws && end_no_ws > 0)
        {
            while (white_space(s[end_no_ws])) end_no_ws--;
        }

        splits.push_back(s.substr(begin_no_ws, end_no_ws - begin_no_ws + 1));
        begin = end + delimiter.size();
    }
}

bool read_chunk(
    std::istream& source,
    std::string& buffer,
    long source_size,
    long chunk_size,
    long chunk_number
)
{
    auto begin = chunk_number * chunk_size;
    if (begin >= source_size) return false;

    auto end = begin + chunk_size;
    if (end > source_size) end = source_size;

    buffer.clear();
    buffer.resize(end - begin);
    source.seekg(begin);
    source.read(buffer.data(), end - begin);

    return true;
}
