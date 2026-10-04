#pragma once
#include <string>
#include <vector>
#include "document.hpp"

struct EvalQuery {
    std::string query;
    std::vector<std::string> relevant_docs;   // filenames, not IDs
};

struct EvalReport {
    size_t num_queries = 0;
    size_t k           = 0;
    double recall_at_k = 0.0;
    double mrr         = 0.0;
};

EvalReport evaluate(const Engine& engine,
                    const std::vector<EvalQuery>& queries,
                    size_t k);

std::vector<EvalQuery> load_eval_queries(const std::string& path);