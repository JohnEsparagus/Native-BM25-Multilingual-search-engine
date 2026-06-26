#pragma once
#include <vector>
#include <string>
#include <string_view>

void fast_lower(std::string& doc);


std::vector<std::string_view> tokenise(std::string& doc);


