#include "headers/evaluation.hpp"
#include <algorithm>

//this eval was done speedily with AI

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

        // ---- Recall@k ----
        // hits = number of top-k results that are in the relevant set.
        size_t hits = 0;
        for (const auto& r : ranked) {
            if (std::find(eq.relevant_docs.begin(),
                          eq.relevant_docs.end(),
                          r.doc_id) != eq.relevant_docs.end()) {
                ++hits;
            }
        }
        if (!eq.relevant_docs.empty()) {
            recall_sum += static_cast<double>(hits) /
                          static_cast<double>(eq.relevant_docs.size());
        }

        // ---- MRR ----
        // Reciprocal rank = 1 / (rank of first relevant hit), else 0.
        for (size_t rank = 0; rank < ranked.size(); ++rank) {
            if (std::find(eq.relevant_docs.begin(),
                          eq.relevant_docs.end(),
                          ranked[rank].doc_id) != eq.relevant_docs.end()) {
                rr_sum += 1.0 / static_cast<double>(rank + 1);
                break;   // only the FIRST hit counts for RR
            }
        }
    }

    report.recall_at_k = recall_sum / static_cast<double>(queries.size());
    report.mrr         = rr_sum     / static_cast<double>(queries.size());
    return report;
}