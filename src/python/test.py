import sys
from pathlib import Path

import numpy as np
from sentence_transformers import SentenceTransformer

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "src" / "python"))            # for qrels
sys.path.insert(0, str(ROOT / "build-python" / "python")) 
from qrels import load_qrels
  # for multilingual_search.so         
from multilingual_search import SearchEngine

K = 10
CORPUS_DIR = ROOT / "corpus" / "english_docs"
QRELS_PATH = ROOT / "eval" / "english.tsv"

# ---- corpus: title = filename, same as TextLoader ----
docs = {
    p.name: p.read_text(encoding="utf-8", errors="replace")
    for p in sorted(CORPUS_DIR.iterdir()) if p.is_file()
}
titles = list(docs)
qrels = load_qrels(QRELS_PATH)

# ---- metrics ----
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

# ---- BM25 (C++) ----
engine = SearchEngine("english")
for t in titles:
    engine.add_document(docs[t], t)

def bm25_search(q):
    return [r.title for r in engine.search(q, K)]

# ---- dense ----
model = SentenceTransformer("paraphrase-multilingual-MiniLM-L12-v2")

def embed(texts):
    e = model.encode(texts, convert_to_numpy=True, show_progress_bar=False)
    return e / np.linalg.norm(e, axis=1, keepdims=True)

doc_emb = embed([docs[t] for t in titles])       # NAIVE: only first 128 tokens of each book

def dense_search(q):
    scores = doc_emb @ embed([q])[0]
    return [titles[i] for i in np.argsort(-scores)[:K]]

# ---- run ----
print(f"{len(titles)} docs, {len(qrels)} queries, K={K}")
for name, fn in [("BM25 (C++)", bm25_search), ("Dense (naive)", dense_search)]:
    r, m = evaluate(fn, qrels)
    print(f"{name:<14} Recall@{K}: {r:.3f}   MRR: {m:.3f}")