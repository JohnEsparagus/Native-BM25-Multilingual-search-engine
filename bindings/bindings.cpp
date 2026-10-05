#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "headers/document.hpp"
#include "headers/Storage.hpp"

namespace py = pybind11;

namespace {

// Python-facing result: SearchResult only has doc_id + score, so add the title.
struct SearchHit {
    size_t      doc_id;
    std::string title;
    double      score;
};

Language parse_language(const std::string& name) {
    if (name == "english") return Language::English;
    if (name == "chinese") return Language::Chinese;
    if (name == "arabic")  return Language::Arabic;
    throw std::invalid_argument(
        "Unknown language '" + name + "' (expected 'english', 'chinese' or 'arabic')");
}

} // namespace

PYBIND11_MODULE(multilingual_search, m) {
    m.doc() = "Python interface to the native C++ BM25 multilingual search engine";

    // std::invalid_argument -> ValueError, std::runtime_error -> RuntimeError
    // (pybind11 translates these automatically).

    py::class_<SearchHit>(m, "SearchResult")
        .def_readonly("doc_id", &SearchHit::doc_id)
        .def_readonly("title",  &SearchHit::title)
        .def_readonly("score",  &SearchHit::score)
        .def("__repr__", [](const SearchHit& h) {
            return "SearchResult(doc_id=" + std::to_string(h.doc_id) +
                   ", title='" + h.title +
                   "', score=" + std::to_string(h.score) + ")";
        });

    py::class_<Engine>(m, "SearchEngine")
        .def(py::init([](const std::string& language) {
                 auto engine = std::make_unique<Engine>();
                 engine->set_language(parse_language(language));
                 return engine;
             }),
             py::arg("language") = "english")

        .def("add_document",
             [](Engine& e, const std::string& text, const std::string& title) {
                 e.add_doc(text, title);
             },
             py::arg("text"), py::arg("title"))

        .def("search",
             [](const Engine& e, const std::string& query, long long top_k) {
                 if (top_k < 0)
                     throw std::invalid_argument("top_k must be >= 0");
                 std::vector<SearchHit> hits;
                 for (const auto& r : e.query(query)) {   // already ranked, score > 0
                     if (hits.size() >= static_cast<size_t>(top_k)) break;
                     hits.push_back({r.doc_id, e.title_of(r.doc_id), r.score});
                 }
                 return hits;
             },
             py::arg("query"), py::arg("top_k") = 10)

        .def("save",
             [](const Engine& e, const std::string& path) { Storage::save(e, path); },
             py::arg("path"))

        .def("load",
             [](Engine& e, const std::string& path) { Storage::load(e, path); },
             py::arg("path"))

        .def("clear", &Engine::clear)
        .def("__len__", &Engine::total_docs);
}