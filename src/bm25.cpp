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
    std::filesystem::path path = "/home/john/Coding/Native-BM25-Multilingual-search-engine/corpus";
    std::filesystem::path index_path = "storage/index.bin";
    
    if (std::filesystem::exists(index_path) && !std::filesystem::is_empty(index_path))// && Storage::is_valid_index(index_path) doesnt work idk why
    {
        std::cout << "Loading existing index...\n";
        Storage::load(engine, index_path);
    }
    else
    {
        std::cout << "Index not found or invalid. Building new index...\n";
        TextLoader loader;
        loader.load_codex(path, engine);
        Storage::save(engine, index_path);
        }
    std::string input;
    while (true) {
        std::cout << "Enter something: ";
        std::getline(std::cin, input); 

        // Pass the user's input clearvariable into the function
        engine.print_query(input);
        std::cout << "You typed: " << input << "\n";
    }

}