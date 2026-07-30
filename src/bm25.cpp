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



int main(){
    Engine engine;
    TextLoader loader;
    std::filesystem::path path = "/home/john/Coding/Native-BM25-Multilingual-search-engine/corpus";
    loader.load_codex(path, engine);
    //engine.print_engine();


    //engine.print_engine();
    


    


}