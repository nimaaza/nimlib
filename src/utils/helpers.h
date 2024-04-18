#pragma once

#include <vector>
#include <string>
#include <string_view>

void split(const std::string_view s,const std::string& delimiter,std::vector<std::string_view>& splits);
