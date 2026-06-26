#include "headers/bm25.hpp"


std::vector<std::string> corpus = {
    "The quick brown fox jumps over the lazy dog",
    "A brown fox is fast and furious, but the lazy dog is sleeping in the grass today",
    "Foxes are very fast animals. The quick fox loves jumping over lazy dogs in the yard",
    "The quick brown fox"};

int main(){
    for (auto& doc : corpus){
        auto tokens = tokenise(doc);
        std::cout << "Doc: " << doc << "\n";
        std::cout << "Tokens: ";
        for (auto& t : tokens) std::cout << "[" << t << "] ";
        std::cout << "\n\n";        
    }

}