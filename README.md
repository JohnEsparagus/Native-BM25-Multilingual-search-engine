# Native-BM25-Multilingual-search-engine


C++20 BM25 search engine (English, Chinese; Arabic is a stub) with an optional pybind11 Python interface.

## Prerequisites
- C++20 compiler, CMake >= 3.18, Python >= 3.8
- Snowball `libstemmer.a` at `third_party/snowball/libstemmer.a` (headers in `third_party/snowball/include`), built with `-fPIC`
- cppjieba at `third_party/cppjieba`

```bash
git submodule update --init --recursive
cd snowball && make CFLAGS="-O2 -fPIC" libstemmer.a && cd ..
# make sure libstemmer.a and the headers end up under third_party/snowball/
```

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