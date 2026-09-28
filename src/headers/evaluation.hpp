#pragma once
#include <string>
#include <vector>
#include "document.hpp"   // for SearchResult, Engine, Language

struct EvalQuery {
    std::string query;
    std::vector<size_t> relevant_docs;   // ground-truth doc ids
};

struct EvalReport {
    size_t num_queries = 0;
    size_t k           = 0;
    double recall_at_k = 0.0;
    double mrr         = 0.0;
};

// Runs each query through the engine, computes Recall@k and MRR, returns averages.
EvalReport evaluate(const Engine& engine,
                    const std::vector<EvalQuery>& queries,
                    size_t k);