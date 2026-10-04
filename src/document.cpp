#include "headers/document.hpp"
#include "headers/tokeniser.hpp"
#include "headers/tokeniserchinese.hpp"
#include "headers/tokeniserarabic.hpp"

Document::Document(const std::vector<std::string_view>& tokens, size_t doc_id, std::string title){
    id = doc_id;
    len = tokens.size();
    this->title = std::move(title); //

    for (auto& token : tokens){
        term_freq[std::string(token)]++;
    }
}

void Engine::add_doc(const std::string& text, std::string title)
{
    size_t new_id  = docs_.size();
    std::vector<std::string> tokens;

    if (this->current_language == Language::English ){
        tokens = tokenise_and_stem(text); //std vec string
    } else if (this->current_language == Language::Chinese ){
        tokens = tokenise_chinese(text);
    }

    std::vector<std::string_view> tokens_view(tokens.begin(), tokens.end());

    docs_.emplace_back(std::move(tokens_view), new_id, std::move(title));
    index_.add_doc(docs_.back());

}

void Engine::build_index(std::vector<std::string>& corpus){

    docs_.clear();
    docs_.reserve(corpus.size());
    
    for (auto& text : corpus){
        add_doc(text, "test"); 
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
    print_query("fox");
    std::cout << "=====================\n";
}

void Engine::clear(){
    docs_.clear();
    index_.clear();
}

void Engine::print_query(const std::string& query_text) const
{

    auto results = query(query_text);
    

    std::cout << "\n=== Query ===\n";
    std::cout << "Query: \"" << query_text << "\"\n\n";

    for (const auto& r : results) {
        std::cout << title_of(r.doc_id)
                    << " (Doc " << r.doc_id << ")"
                    << " -> Score: " << r.score << '\n';
    
    }

    std::cout << "=====================\n";
}
    std::vector<SearchResult> Engine::query(const std::string &query_text) const
{
    Scorer scorer(index_);
    std::vector<std::string> tokens ;
        if (this->current_language == Language::English ){
        tokens = tokenise_and_stem(query_text); //std vec string
    } else if (this->current_language == Language::Chinese ){
        tokens = tokenise_chinese(query_text);
    }else if (this->current_language == Language::Arabic ){
        tokens = tokenise_arabic(query_text);
    }

    std::vector<double> scores(index_.total_docs(),0.0);

    for (const auto& token : tokens){
        const auto* postings = index_.get_postings(token);

        if (!postings){
            continue;
        }

        for (const auto& [id, freq] : *postings){
            scores[id] += scorer.score_term(token, freq, index_.doc_len[id]);
        }
    }

    std::vector<SearchResult> results;
    results.reserve(scores.size());

    for (size_t id = 0; id < scores.size(); id++){
        if (scores[id] > 0.0){
            results.push_back({id, scores[id]});
        }
    }

  std::sort(results.begin(), results.end(),
              [](const SearchResult& a, const SearchResult& b) {
                  return a.score > b.score;
              });

    return results;
}

void InvertedIndex::clear()
{
    index.clear();
    doc_len.clear();
    total_len = 0;
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

Scorer::Scorer(const InvertedIndex& idx) : index(idx){}

#include <iostream> // Ensure this header is included at the top of your file

double Scorer::score_term(const std::string &term, size_t term_freq_in_doc, size_t doc_len) const {     
    double k1 = 1.2;     
    double b = 0.75;      
    
    double N = index.total_docs();     
    double df = index.get_doc_freq(term);     
    double avgdl = index.get_avg_doc_len();      
    

    if (df == 0){         
        return 0.0;     
    }      

    const double idf = std::log(1.0 + (N - df + 0.5) / (df + 0.5));      
    const double length_normalization = (1 - b) + b * (doc_len / avgdl);      
    const double tf_saturation = (term_freq_in_doc * (k1 + 1)) / (term_freq_in_doc + k1 * length_normalization);        
    
    double final_score = idf * tf_saturation;


    return final_score; 
}


namespace fs = std::filesystem;
void TextLoader::load_codex(std::filesystem::path& path, Engine& engine) {
    try {
        if (fs::exists(path) && fs::is_directory(path)){

            std::vector<fs::path> files;
            for (const auto& text :fs::directory_iterator(path)){
                if (fs::is_regular_file(text.path())){
                    files.push_back(text.path());
                }

                std::sort(files.begin(), files.end());

            }

            for (const auto& text : files){


                
                if (fs::is_regular_file(text)){
                    std::ifstream file(text, std::ios::binary | std::ios::ate);

                    if (file.is_open()){
                        //fill the vector
                        std::streampos size = file.tellg();;
                        std::string content;
                        content.resize(size);
                        file.seekg(0, std::ios::beg);
                        if (file.read(content.data(), size)){
                            engine.add_doc(content, text.filename().string());
                        }
                    }
                    else{
                       std::cerr << " failed To oepen " <<"\n";    
                    }
                }

            }
        }
        else{
            std::cout << "path is not a directory";
        }
    }   catch (const fs::filesystem_error& e){
        std::cerr << "Filesystem error: " << e.what() << "\n";        //error
    }
}

