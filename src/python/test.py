import sys
from pathlib import Path

import numpy as np

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "src" / "python"))            # for qrels
from qrels import load_qrels
from search import Searcher

LANGUAGE = sys.argv[1] if len(sys.argv) > 1 else "english"

K = 3
QRELS_PATHS = [ROOT / "eval" / f"{LANGUAGE}.tsv", ROOT / "eval" / f"{LANGUAGE}_thematic.tsv"]

searcher = Searcher(LANGUAGE)

def books_in_rank_order(ranked_passages):
    return list(dict.fromkeys(searcher.passage_book[i] for i in ranked_passages))[:K]

def recall_at_k(ranked, relevant, k):
    return len(set(ranked[:k]) & relevant) / len(relevant)

def mrr(ranked, relevant, k):
    for i, t in enumerate(ranked[:k], start=1):
        if t in relevant:
            return 1.0 / i
    return 0.0

def evaluate(search_fn, qrels):
    r, m = [], []
    for q, rel in qrels:
        ranked = search_fn(q)
        r.append(recall_at_k(ranked, rel, K))
        m.append(mrr(ranked, rel, K))
    return np.mean(r), np.mean(m)

print(f"{LANGUAGE}: {len(searcher.docs)} books, {len(searcher.passages)} passages, K={K}")
for path in QRELS_PATHS:
    qrels = load_qrels(path)
    print(f"\n{path.name}: {len(qrels)} queries")
    for name, fn in [("BM25 (C++)", searcher.bm25), ("Dense", searcher.dense), ("Hybrid (RRF)", searcher.hybrid)]:
        r, m = evaluate(lambda q: books_in_rank_order(fn(q)), qrels)
        print(f"{name:<14} Recall@{K}: {r:.3f}   MRR: {m:.3f}")
