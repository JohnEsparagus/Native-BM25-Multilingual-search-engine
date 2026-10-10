import hashlib
import re
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

LANGUAGE = sys.argv[1] if len(sys.argv) > 1 else "english"

CHUNK_CHARS = {"english": 500, "chinese": 150, "arabic": 400}[LANGUAGE]

RRF_K = 60
RRF_DEPTH = 100

K = 3
CORPUS_DIR = ROOT / "corpus" / f"{LANGUAGE}_docs"
QRELS_PATHS = [ROOT / "eval" / f"{LANGUAGE}.tsv", ROOT / "eval" / f"{LANGUAGE}_thematic.tsv"]

docs = {
    p.name: p.read_text(encoding="utf-8", errors="replace")
    for p in sorted(CORPUS_DIR.iterdir()) if p.is_file()
}

def chunk(text, size):
    chunks, cur = [], ""
    for para in re.split(r"\n\s*\n", text):
        para = " ".join(para.split())
        for i in range(0, len(para), size):
            piece = para[i:i + size]
            if cur and len(cur) + len(piece) > size:
                chunks.append(cur)
                cur = ""
            cur = (cur + " " + piece).strip()
    if cur:
        chunks.append(cur)
    return chunks

passages = []
passage_book = []
for title, text in docs.items():
    for c in chunk(text, CHUNK_CHARS):
        passages.append(c)
        passage_book.append(title)

def books_in_rank_order(ranked_passages):
    return list(dict.fromkeys(passage_book[i] for i in ranked_passages))[:K]

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

engine = SearchEngine(LANGUAGE)
for text, book in zip(passages, passage_book):
    engine.add_document(text, book)

def bm25_passages(q):
    return [r.doc_id for r in engine.search(q, len(passages))]

model = SentenceTransformer("paraphrase-multilingual-MiniLM-L12-v2")

def embed(texts, progress=False):
    e = model.encode(texts, convert_to_numpy=True, show_progress_bar=progress, batch_size=64)
    return e / np.linalg.norm(e, axis=1, keepdims=True)

corpus_hash = hashlib.md5("\n".join(passages).encode("utf-8")).hexdigest()[:8]
EMB_PATH = ROOT / "storage" / f"embeddings_{LANGUAGE}_{corpus_hash}.npy"

if EMB_PATH.exists():
    print(f"Loading cached embeddings from {EMB_PATH.name}")
    doc_emb = np.load(EMB_PATH)
else:
    print(f"Embedding {len(passages)} passages...")
    doc_emb = embed(passages, progress=True)
    np.save(EMB_PATH, doc_emb)

def dense_passages(q):
    scores = doc_emb @ embed([q])[0]
    return list(np.argsort(-scores))

def hybrid_passages(q):
    fused = {}
    for ranked in (bm25_passages(q), dense_passages(q)):
        for rank, i in enumerate(ranked[:RRF_DEPTH], start=1):
            fused[i] = fused.get(i, 0.0) + 1.0 / (RRF_K + rank)
    return sorted(fused, key=fused.get, reverse=True)

print(f"{LANGUAGE}: {len(docs)} books, {len(passages)} passages, K={K}")
for path in QRELS_PATHS:
    qrels = load_qrels(path)
    print(f"\n{path.name}: {len(qrels)} queries")
    for name, fn in [("BM25 (C++)", bm25_passages), ("Dense", dense_passages), ("Hybrid (RRF)", hybrid_passages)]:
        r, m = evaluate(lambda q: books_in_rank_order(fn(q)), qrels)
        print(f"{name:<14} Recall@{K}: {r:.3f}   MRR: {m:.3f}")
