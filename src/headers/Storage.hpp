#pragma once

#include <fstream>
#include <unordered_map>
#include <string>
#include "document.hpp"
#include <cstring>
#include <array>

class Storage {
public:
    static void save(const Engine& engine, const std::string& filename);
    static void load(Engine& engine, const std::string& filename);
    static bool is_valid_index(const std::string& filename);
};