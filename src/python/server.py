import threading
from collections import Counter
from pathlib import Path

from fastapi import FastAPI, HTTPException, Query
from fastapi.responses import FileResponse
from pydantic import BaseModel

from search import CHUNK_CHARS, ROOT, Searcher, corpus_dir, cross_lingual

app = FastAPI(title="BM25 / Hybrid Multilingual Retrieval")

searchers = {}
lock = threading.Lock()


def check_language(language):
    if language not in CHUNK_CHARS:
        raise HTTPException(400, f"Unknown language '{language}'")


def get_searcher(language):
    check_language(language)
    with lock:
        if language not in searchers:
            searchers[language] = Searcher(language)
        return searchers[language]


def document_path(language, title):
    name = Path(title).name
    if not name or name.startswith("."):
        raise HTTPException(400, "Invalid document title")
    if not name.endswith(".txt"):
        name += ".txt"
    return corpus_dir(language) / name


@app.get("/")
def index():
    return FileResponse(ROOT / "web" / "index.html")


@app.get("/search")
def search(q: str, language: str = "english", k: int = Query(5, ge=1, le=20)):
    if language == "all":
        every = {name: get_searcher(name) for name in CHUNK_CHARS}
        dense = [
            {"book": every[name].passage_book[i], "text": every[name].passages[i], "language": name}
            for _, name, i in cross_lingual(every, q, k)
        ]
        return {"bm25": None, "dense": dense, "hybrid": None}

    searcher = get_searcher(language)

    def hits(ranked):
        return [{"book": searcher.passage_book[i], "text": searcher.passages[i]} for i in ranked[:k]]

    return {
        "bm25": hits(searcher.bm25(q)),
        "dense": hits(searcher.dense(q)),
        "hybrid": hits(searcher.hybrid(q)),
    }


@app.get("/documents")
def list_documents(language: str = "english"):
    searcher = get_searcher(language)
    passages = Counter(searcher.passage_book)
    return [{"title": title, "passages": passages[title]} for title in searcher.docs]


class NewDocument(BaseModel):
    language: str
    title: str
    text: str


@app.post("/documents")
def add_document(doc: NewDocument):
    check_language(doc.language)
    if not doc.text.strip():
        raise HTTPException(400, "Document is empty")
    path = document_path(doc.language, doc.title)

    with lock:
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(doc.text, encoding="utf-8")
        searchers[doc.language] = Searcher(doc.language)
    return {"title": path.name}


@app.delete("/documents")
def remove_document(title: str, language: str = "english"):
    searcher = get_searcher(language)
    path = document_path(language, title)
    if not path.is_file():
        raise HTTPException(404, f"No document '{path.name}'")

    with lock:
        path.unlink()
        embedding = searcher.embedding_paths.get(path.name)
        if embedding is not None:
            embedding.unlink(missing_ok=True)
        searchers[language] = Searcher(language)
    return {"title": path.name}
