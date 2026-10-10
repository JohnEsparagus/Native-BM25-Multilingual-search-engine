# ---- stage 1: compile the C++ engine and its Python module ----
FROM python:3.12-slim AS build

RUN apt-get update \
    && apt-get install -y --no-install-recommends build-essential cmake perl \
    && rm -rf /var/lib/apt/lists/*
RUN pip install --no-cache-dir pybind11==3.1.0

WORKDIR /app
COPY CMakeLists.txt .
COPY src src
COPY bindings bindings
COPY third_party third_party

RUN cmake -S . -B build-python \
        -DBUILD_PYTHON_BINDINGS=ON \
        -Dpybind11_DIR="$(python -m pybind11 --cmakedir)" \
    && cmake --build build-python --parallel --target multilingual_search


# ---- stage 2: the search server, no compiler ----
FROM python:3.12-slim

WORKDIR /app

# CPU-only torch, the default wheel pulls in several GB of CUDA libraries
COPY requirements.txt .
RUN pip install --no-cache-dir torch==2.14.1 --index-url https://download.pytorch.org/whl/cpu \
    && pip install --no-cache-dir -r requirements.txt

COPY --from=build /app/build-python/python build-python/python
COPY --from=build /app/third_party/cppjieba/dict third_party/cppjieba/dict
COPY corpus corpus
COPY storage storage
COPY src/python src/python
COPY web web

# Downloads the dense model and embeds every passage, so the container starts
# ready and works offline. Embeddings already in storage/ are reused.
RUN python src/python/search.py
ENV HF_HUB_OFFLINE=1

EXPOSE 8000
CMD ["uvicorn", "server:app", "--app-dir", "src/python", "--host", "0.0.0.0", "--port", "8000"]
