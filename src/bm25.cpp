#include "headers/bm25.hpp"
#include "headers/document.hpp"


std::vector<std::string> corpus = {
    "The quick brown fox jumps over the lazy dog",
    "A brown fox is fast and furious, but the lazy dog is sleeping in the grass today",
    "Foxes are very fast animals. The quick fox loves jumping over lazy dogs in the yard",
    "The quick brown fox"};
//assume corpus is like, every text is a file, but rn a string


int idf_score(){
    return 0;
}

int length_norm(){
    return 0;
}

int tf_boost(){
    return 0;
}

int bm25_value(const std::string& term, size_t term_freq_in_doc, size_t doc_len){
    
    return 0;
}

int main(){
    Engine engine;
    engine.build_index(corpus);
    engine.print_engine();


    


}