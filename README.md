# Native-BM25-Multilingual-search-engine


C++20 BM25 search engine (English, Chinese, Arabic) with an optional pybind11 Python interface.

## Prerequisites
- C++20 compiler, CMake >= 3.18, make, perl, Python >= 3.8
- Snowball and cppjieba are included under `third_party/`. CMake builds `libstemmer.a` on the first build.

## Native build (unchanged)
```bash
./run.sh
```

## Python extension
```bash
python3 -m venv .venv
source .venv/bin/activate
pip install pybind11 pytest

cmake -S . -B build \
  -DBUILD_PYTHON_BINDINGS=ON \
  -DPython_EXECUTABLE="$(which python)" \
  -Dpybind11_DIR="$(python -m pybind11 --cmakedir)"
cmake --build build --parallel
```
The module is written to `build/python/multilingual_search*.so`.

## Python smoke test
```bash
PYTHONPATH=build/python pytest tests/python -v
```

## Usage
```python
from multilingual_search import SearchEngine

engine = SearchEngine()            # or SearchEngine("chinese")
engine.add_document("The whale was spotted near the ship.", "moby_dick")
for r in engine.search("white whale", 10):
    print(r.title, r.score)

engine.save("storage/my_index.bin")   # directory must already exist
engine.load("storage/my_index.bin")
```

## Evaluation: BM25 vs dense vs hybrid
```bash
pip install -r requirements.txt
python src/python/test.py english     # or chinese, arabic
```
The first run per language embeds every passage (minutes on CPU) and caches the result in `storage/`.

## Web demo
```bash
uvicorn server:app --app-dir src/python
```
Open http://localhost:8000 and search. Results from BM25, dense and hybrid are shown side by side.

Pick "All languages" to search every book with one query, in any language. This uses dense search only, since BM25 needs the query and the text to share words.

Open "Documents" on the page to add your own `.txt` files or remove books. Added files are saved in `corpus/<language>_docs/` and their embeddings in `storage/`, so they are still there after a restart.

## Docker
```bash
docker build -t bm25-search .
docker run -p 8000:8000 -v bm25-corpus:/app/corpus -v bm25-storage:/app/storage bm25-search
```
The image compiles the engine, downloads the dense model and embeds the corpus at build time, so the container runs offline. The two volumes keep documents you add through the page.

To use a different cppjieba dictionary location at runtime, set `JIEBA_DICT_DIR`.
