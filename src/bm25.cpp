#include "headers/bm25.hpp"
#include "headers/document.hpp"


std::vector<std::string> corpus = {
    "The quick brown fox jumps over the lazy dog",
    "A brown fox is fast and furious, but the lazy dog is sleeping in the grass today",
    "Foxes are very fast animals. The quick fox loves jumping over lazy dogs in the yard",
    "The quick brown fox"};
//assume corpus is like, every text is a file, but rn a string




int main(){
    Engine engine;
    engine.build_index(corpus);
    engine.print_engine();

    


}