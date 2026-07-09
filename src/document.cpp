#include "headers/document.hpp"
#include "headers/tokeniser.hpp"
#include "document.hpp"

Document::Document(const std::string& content, size_t doc_id){
    id = doc_id;
    len = 0;

    auto tokens = tokenise(content);

    for (auto& token : tokens){
        term_freq[std::string(token)]++;
        len++;
    }
}

void Engine::build_index(std::vector<std::string> corpus){

    int id = 0;
    size_t total_len = 0;
    for (auto& doc : corpus){
        docs.emplace_back(doc,id);
        id++;

        total_len += docs.back().len;

        for (auto& [term, freq] : docs.back().term_freq) doc_freq[term]++;
    }

    total_docs = docs.size();
    avg_doc_len = static_cast<double>(total_len) / total_docs;
}













//testing 

void Document::print_doc() const {
    std::cout << "=== Document Info ===\n";
    std::cout << "ID: " << id << "\n";
    std::cout << "Length: " << len << "\n";
    std::cout << "Term Frequencies:\n";
    for (const auto& [term, freq] : term_freq) {
        std::cout << "  - " << term << ": " << freq << "\n";
    }
    std::cout << "---------------------\n";
}
void Engine::print_engine(){
    std::cout << "=== Engine Status ===\n";
    std::cout << "Total Docs: " << total_docs << "\n";
    std::cout << "Avg Doc Length: " << avg_doc_len << "\n";
    std::cout << "Doc Frequency Variable: " << doc_frequency << "\n";
    
    std::cout << "\nGlobal Document Frequencies:\n";
    for (const auto& [word, freq] : doc_freq) {
        std::cout << "  - " << word << ": " << freq << "\n";
    }

    std::cout << "\nStored Documents Details:\n";
    for (const auto& doc : docs) {
        doc.print_doc(); // Calls the Document member print function
    }
    std::cout << "=====================\n";
}
std::vector<std::pair<size_t, double>> Engine::query(const std::string &query_text) const
{
    return std::vector<std::pair<size_t, double>>();
}
void InvertedIndex::add_doc(const Document &doc)
{
}

size_t InvertedIndex::get_doc_freq(const std::string &term) const
{
    return size_t();
}

const std::vector<Posting>* get_postings(const std::string& term) const{

}

double Scorer::score_term(const std::string &term, size_t term_freq_in_doc, size_t doc_len) const
{
    return 0.0;
}
