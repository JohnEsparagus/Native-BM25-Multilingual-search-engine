#pragma once
#include <string>
#include <vector>
#include "headers/bm25.hpp"
#include "headers/tokeniser.hpp"
#include <iostream>

int bm25_value(const std::string& term, size_t term_freq_in_doc, size_t doc_len);
