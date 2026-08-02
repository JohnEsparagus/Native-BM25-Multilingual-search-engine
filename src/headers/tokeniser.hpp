#pragma once
#include <vector>
#include <string>
#include <unordered_set>
#include "headers/stemmer.h"

std::string fast_lower( const std::string& doc);


std::vector<std::string> tokenise(const std::string& doc);


std::vector<std::string> tokenise_and_stem(const std::string& doc);
