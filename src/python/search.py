import hashlib
import re
import sys
from pathlib import Path

import numpy as np
from sentence_transformers import SentenceTransformer

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT / "build-python" / "python"))
from multilingual_search import SearchEngine

CHUNK_CHARS = {"english": 500, "chinese": 150, "arabic": 400}
MODEL_NAME = "paraphrase-multilingual-MiniLM-L12-v2"

RRF_K = 60
RRF_DEPTH = 100

_model = None

def embed(texts, progress=False):
    global _model
    if _model is None:
        _model = SentenceTransformer(MODEL_NAME)
    e = _model.encode(texts, convert_to_numpy=True, show_progress_bar=progress, batch_size=64)
    return e / np.linalg.norm(e, axis=1, keepdims=True)

def corpus_dir(language):
    return ROOT / "corpus" / f"{language}_docs"

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


class Searcher:
    def __init__(self, language):
        corpus_dir(language).mkdir(parents=True, exist_ok=True)
        self.docs = {
            p.name: p.read_text(encoding="utf-8", errors="replace")
            for p in sorted(corpus_dir(language).iterdir()) if p.is_file()
        }

        self.passages = []
        self.passage_book = []
        self.embedding_paths = {}
        embeddings = []

        # embeddings are cached per book, so adding or removing one book never re-embeds the rest
        for title, text in self.docs.items():
            chunks = chunk(text, CHUNK_CHARS[language])
            if not chunks:
                continue
            book_hash = hashlib.md5("\n".join(chunks).encode("utf-8")).hexdigest()[:12]
            emb_path = ROOT / "storage" / f"embeddings_{language}_{book_hash}.npy"

            if emb_path.exists():
                emb = np.load(emb_path)
            else:
                print(f"Embedding {len(chunks)} passages of {title}...")
                emb = embed(chunks, progress=True)
                emb_path.parent.mkdir(exist_ok=True)
                np.save(emb_path, emb)

            self.passages.extend(chunks)
            self.passage_book.extend([title] * len(chunks))
            self.embedding_paths[title] = emb_path
            embeddings.append(emb)

        self.doc_emb = np.vstack(embeddings) if embeddings else None

        self.engine = SearchEngine(language)
        for text, book in zip(self.passages, self.passage_book):
            self.engine.add_document(text, book)

    def bm25(self, q):
        return [r.doc_id for r in self.engine.search(q, len(self.passages))]

    def dense(self, q):
        if self.doc_emb is None:
            return []
        scores = self.doc_emb @ embed([q])[0]
        return [int(i) for i in np.argsort(-scores)]

    def hybrid(self, q):
        fused = {}
        for ranked in (self.bm25(q), self.dense(q)):
            for rank, i in enumerate(ranked[:RRF_DEPTH], start=1):
                fused[i] = fused.get(i, 0.0) + 1.0 / (RRF_K + rank)
        return sorted(fused, key=fused.get, reverse=True)


def cross_lingual(searchers, q, k):
    q_emb = embed([q])[0]
    hits = []
    for language, searcher in searchers.items():
        if searcher.doc_emb is None:
            continue
        scores = searcher.doc_emb @ q_emb
        for i in np.argsort(-scores)[:k]:
            hits.append((float(scores[i]), language, int(i)))
    return sorted(hits, reverse=True)[:k]


if __name__ == "__main__":
    for language in CHUNK_CHARS:
        Searcher(language)
