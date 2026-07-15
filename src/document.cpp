#include "headers/document.hpp"
#include "headers/tokeniser.hpp"

Document::Document(const std::vector<std::string_view>& tokens, size_t doc_id){
    id = doc_id;
    len = tokens.size();

    for (auto& token : tokens){
        term_freq[std::string(token)]++;
    }
}

void Engine::add_doc(const std::string& text)
{
    size_t new_id  = docs_.size();
    
    std::vector<std::string> tokens = tokenise(text); //std vec string
    std::vector<std::string_view> tokens_view(tokens.begin(), tokens.end());

    docs_.emplace_back(std::move(tokens_view), new_id);
    index_.add_doc(docs_.back());

}

void Engine::build_index(std::vector<std::string>& corpus){

    docs_.clear();
    docs_.reserve(corpus.size());
    
    for (auto& text : corpus){
        add_doc(text); //engine
    }


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
    std::cout << "Total Docs: " << total_docs() << "\n";
    std::cout << "Avg Doc Length: " << average_doc_len() << "\n";

    std::cout << "\nGlobal Document Frequencies:\n";
    for (const auto& [word, postings] : index_.index) {
        std::cout << "  - " << word << ": " << postings.size() << "\n";
    }

    std::cout << "\nStored Documents Details:\n";
    for (const auto& doc : docs_) {
        doc.print_doc();
    }
    std::cout << "=====================\n";
}
std::vector<std::pair<size_t, double>> Engine::query(const std::string &query_text) const
{
    return std::vector<std::pair<size_t, double>>();
}
void InvertedIndex::add_doc(const Document &doc)
{
    if (doc.id != doc_len.size()){
        throw std::logic_error("InvertedIndex::add_doc: doc.id must equal current doc count");
    }

    doc_len.push_back(doc.len);
    total_len+=doc.len;

    //what doc ids and times the string is references. using tokens
    for (const auto& [word, freq] : doc.term_freq){
        index[word].push_back(Posting{doc.id, freq});
    }
}

size_t InvertedIndex::get_doc_freq(const std::string &term) const
{
    auto it = index.find(term);
    if (it != index.end() ){
        return it->second.size();
    } else {return 0;}
}

 const std::vector<Posting>* InvertedIndex::get_postings(const std::string& term) const{
    auto it = index.find(term);
    if (it != index.end()){
        return &it->second;
    } else {return nullptr;}
}

double Scorer::score_term(const std::string &term, size_t term_freq_in_doc, size_t doc_len) const
{
    return 0.0;
}
