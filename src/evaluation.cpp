#include "headers/evaluation.hpp"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <iostream>

EvalReport evaluate(const Engine& engine,
                    const std::vector<EvalQuery>& queries,
                    size_t k)
{
    EvalReport report;
    report.num_queries = queries.size();
    report.k           = k;
    if (queries.empty()) return report;

    double recall_sum = 0.0;
    double rr_sum     = 0.0;

    for (const auto& eq : queries) {
        // query() already returns ranked, score>0 results.
        auto ranked = engine.query(eq.query);
        if (ranked.size() > k) ranked.resize(k);   // only top-k matter

        // relevant_docs holds filenames, so compare titles, not IDs
        auto is_relevant = [&](size_t doc_id) {
            const std::string& title = engine.title_of(doc_id);
            return std::find(eq.relevant_docs.begin(),
                             eq.relevant_docs.end(),
                             title) != eq.relevant_docs.end();
        };

        // ---- Recall@k ----
        size_t hits = 0;
        for (const auto& r : ranked) {
            if (is_relevant(r.doc_id)) ++hits;
        }
        if (!eq.relevant_docs.empty()) {
            recall_sum += static_cast<double>(hits) /
                          static_cast<double>(eq.relevant_docs.size());
        }

        // ---- MRR ----
        // Reciprocal rank = 1 / (rank of first relevant hit), else 0.
        for (size_t rank = 0; rank < ranked.size(); ++rank) {
            if (is_relevant(ranked[rank].doc_id)) {
                rr_sum += 1.0 / static_cast<double>(rank + 1);
                break;   // only the FIRST hit counts for RR
            }
        }
    }

    report.recall_at_k = recall_sum / static_cast<double>(queries.size());
    report.mrr         = rr_sum     / static_cast<double>(queries.size());
    return report;
}

static std::string trim(const std::string& s) {
    size_t a = s.find_first_not_of(" \t\r\n");
    if (a == std::string::npos) return "";
    size_t b = s.find_last_not_of(" \t\r\n");
    return s.substr(a, b - a + 1);
}

std::vector<EvalQuery> load_eval_queries(const std::string& path) {
    std::vector<EvalQuery> out;
    std::ifstream in(path);
    if (!in) {
        std::cerr << "Could not open eval file: " << path << "\n";
        return out;
    }

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t arrow = line.find("=>");
        if (arrow == std::string::npos) continue;

        EvalQuery q;
        q.query = trim(line.substr(0, arrow));

        std::stringstream ss(line.substr(arrow + 2));
        std::string name;
        while (std::getline(ss, name, '|')) {
            name = trim(name);
            if (!name.empty()) q.relevant_docs.push_back(name);
        }
        if (!q.query.empty() && !q.relevant_docs.empty())
            out.push_back(std::move(q));
    }
    return out;
}