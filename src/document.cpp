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
        add_doc(text); 
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
    auto scores = query(query_text);

    std::cout << "\n=== Query ===\n";
    std::cout << "Query: \"" << query_text << "\"\n\n";

    for (size_t id = 0; id < scores.size(); ++id)
    {
        if (scores[id] > 0.0)
        {
            std::cout << "Doc " << id
                      << " -> Score: "
                      << scores[id] << '\n';
        }
    }

    std::cout << "=====================\n";
}

    std::vector<double> Engine::query(const std::string &query_text) const
{

    Scorer scorer(index_);
    std::vector<std::string> tokens = tokenise(query_text);

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
    
    return scores;
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

double Scorer::score_term(const std::string &term, size_t term_freq_in_doc, size_t doc_len) const
{
    double k1 = 1.2;
    double b = 0.75;

    double N = index.total_docs();
    double df = index.get_doc_freq(term);
    double avgdl = index.get_avg_doc_len();

    if (df == 0){
        return 0.0;
    }

    const double idf = std::log(1.0+ (N-df + 0.5)/(df + 0.5));

    const double length_normalization = (1-b)+ b*(doc_len / avgdl);

    const double tf_saturation = (term_freq_in_doc *  (k1+1)) / (term_freq_in_doc + k1 * length_normalization);



    return idf *  tf_saturation ;
}

namespace fs = std::filesystem;
void TextLoader::load_codex(std::filesystem::path& path, Engine& engine) {
    try {
        if (fs::exists(path) && fs::is_directory(path)){
            for (const auto& text :fs::directory_iterator(path)){
                
                if (fs::is_regular_file(text.path())){
                    std::ifstream file(text.path(), std::ios::binary | std::ios::ate);

                    if (file.is_open()){
                        //fill the vector
                        std::streampos size = file.tellg();;
                        std::string content;
                        content.resize(size);
                        file.seekg(0, std::ios::beg);
                        if (file.read(content.data(), size)){
                            engine.add_doc(content);
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

