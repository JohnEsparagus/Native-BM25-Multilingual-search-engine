import json
import os
import threading
import time
from collections import Counter
from pathlib import Path

from fastapi import FastAPI, HTTPException, Query, Request
from fastapi.responses import FileResponse, StreamingResponse
from pydantic import BaseModel, Field

from chat import answer
from search import CHUNK_CHARS, ROOT, Searcher, TextSearcher, corpus_dir, cross_lingual, embed

# load the dense model at startup, so the first search's time measures searching, not loading
embed(["warm up"])

app = FastAPI(title="BM25 / Hybrid Multilingual Retrieval")

searchers = {}
lock = threading.Lock()

# set READ_ONLY=1 on a public demo so visitors can search but not change the corpus
READ_ONLY = os.environ.get("READ_ONLY") == "1"

# the chat only shows up when there is an API key to answer with
CHAT_ENABLED = bool(os.environ.get("ANTHROPIC_API_KEY"))
CHAT_PASSAGES = 8
CHAT_LIMIT = 20  # questions per visitor per hour, since every answer costs money
chat_log = {}


def check_editable():
    if READ_ONLY:
        raise HTTPException(403, "Adding and removing documents is turned off in this demo")


def check_chat_limit(request):
    # behind Cloud Run the visitor's address is the first entry of x-forwarded-for
    visitor = request.headers.get("x-forwarded-for", request.client.host).split(",")[0].strip()
    now = time.time()
    recent = [t for t in chat_log.get(visitor, []) if now - t < 3600]
    if len(recent) >= CHAT_LIMIT:
        raise HTTPException(429, "Too many questions, please try again later")
    chat_log[visitor] = recent + [now]


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


@app.get("/settings")
def settings():
    return {"editable": not READ_ONLY, "chat": CHAT_ENABLED}


def elapsed_ms(start):
    return round((time.perf_counter() - start) * 1000, 1)


def ranked_hits(searcher, q, k):
    """Top k passages from each ranker, plus how long each ranker took."""
    results, ms = {}, {}
    for name, rank in (("bm25", searcher.bm25), ("dense", searcher.dense), ("hybrid", searcher.hybrid)):
        start = time.perf_counter()
        ranked = rank(q)
        ms[name] = elapsed_ms(start)
        results[name] = [{"id": i, "book": searcher.passage_book[i], "text": searcher.passages[i]} for i in ranked[:k]]
    return {**results, "ms": ms}


@app.get("/search")
def search(q: str, language: str = "english", k: int = Query(5, ge=1, le=20)):
    if language == "all":
        every = {name: get_searcher(name) for name in CHUNK_CHARS}
        start = time.perf_counter()
        found = cross_lingual(every, q, k)
        ms = elapsed_ms(start)
        dense = [
            {"id": i, "book": every[name].passage_book[i], "text": every[name].passages[i], "language": name}
            for _, name, i in found
        ]
        return {"bm25": None, "dense": dense, "hybrid": None, "ms": {"dense": ms}}

    return ranked_hits(get_searcher(language), q, k)


@app.get("/context")
def context(id: int, language: str = "english", around: int = Query(2, ge=0, le=5)):
    """The passages just before and after one result, from the same book."""
    searcher = get_searcher(language)
    if not 0 <= id < len(searcher.passages):
        raise HTTPException(404, "No such passage")
    book = searcher.passage_book[id]
    nearby = [i for i in range(id - around, id + around + 1)
              if 0 <= i < len(searcher.passages) and searcher.passage_book[i] == book]
    return {"book": book, "passages": [{"text": searcher.passages[i], "match": i == id} for i in nearby]}


class SandboxQuery(BaseModel):
    language: str
    q: str = Field(min_length=1, max_length=500)
    text: str = Field(min_length=1, max_length=20000)  # small enough to embed in a few seconds on one CPU


@app.post("/sandbox")
def sandbox(query: SandboxQuery):
    """Searches text the visitor pasted, without touching the corpus. Allowed in read-only mode."""
    check_language(query.language)
    searcher = TextSearcher(query.language, query.text)
    if not searcher.passages:
        raise HTTPException(400, "The text is empty")
    return ranked_hits(searcher, query.q, 5)


@app.get("/chat")
def chat(request: Request, q: str = Query(..., min_length=1, max_length=500), language: str = "english"):
    if not CHAT_ENABLED:
        raise HTTPException(503, "Chat is not set up on this server")
    check_chat_limit(request)

    if language == "all":
        every = {name: get_searcher(name) for name in CHUNK_CHARS}
        hits = [
            {"id": i, "book": every[name].passage_book[i], "text": every[name].passages[i], "language": name}
            for _, name, i in cross_lingual(every, q, CHAT_PASSAGES)
        ]
    else:
        searcher = get_searcher(language)
        hits = [{"id": i, "book": searcher.passage_book[i], "text": searcher.passages[i]}
                for i in searcher.hybrid(q)[:CHAT_PASSAGES]]

    # first line is the passages as JSON so the page can show sources, then the answer streams
    def body():
        yield json.dumps(hits, ensure_ascii=False) + "\n"
        yield from answer(q, hits)

    return StreamingResponse(body(), media_type="text/plain; charset=utf-8")


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
    check_editable()
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
    check_editable()
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
