#include "headers/bm25.hpp"
#include "headers/document.hpp"
#include "headers/evaluation.hpp"  

std::vector<std::string> corpus = {
    "The quick brown fox jumps over the lazy dog",
    "A brown fox is fast and furious, but the lazy dog is sleeping in the grass today",
    "Foxes are very fast animals. The quick fox loves jumping over lazy dogs in the yard",
    "The quick brown fox"};
//assume corpus is like, every text is a file, but rn a string




int main(){
    Engine engine;
    std::filesystem::path path = "./corpus";
    // path could be english, chinese or etc

    std::cout << "Select which language you would like to parse...\n 1 = English.\n 2 = Chinese.\n 3 = Arabic.\n";
    char choice = 0;

    while (std::cin>>choice && (choice < '1' || choice > '3')){
    
        std::cout<<"Invalid input, choose between the following:\n 1 (English),\n 2 (Chinese),\n 3 (Arabic)";
    }

    std::filesystem::path index_path_english = "storage/index_english.bin";
    std::filesystem::path index_path_chinese = "storage/index_chinese.bin";
    std::filesystem::path index_path_arabic = "storage/index_arabic.bin";

    std::filesystem::path index_path = {};

    if (choice == '1'){
        path /= "english_docs";
        index_path = index_path_english;
        engine.set_language(Language::English);

    } else if (choice == '2'){
        path /= "chinese_docs";
        index_path = index_path_chinese;
        engine.set_language(Language::Chinese);

    } else if (choice == '3'){
        path /= "arabic_docs";
        index_path = index_path_arabic;
        engine.set_language(Language::Arabic);

    }


    
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

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::cout << "hoose between the following:\n 1 (Interactive query),\n 2 (Run Eval)\n";
    char mode = 1;

    while (std::cin>>mode && (mode < '1' || mode > '2')){
    
        std::cout<<"Invalid input, choose between the following:\n 1 (Interactive query),\n 2 (Run Eval) \n";
    }


    if (mode == '1'){
        

    } else if (mode == '2'){
        std::string eval_path =
        choice == '1' ? "eval/english.tsv" :
        choice == '2' ? "eval/chinese.tsv" :
                        "eval/arabic.tsv";

        auto eval_queries = load_eval_queries(eval_path);

        // catch typos in filenames
        for (const auto& q : eval_queries)
        for (const auto& name : q.relevant_docs) {
            bool found = false;
            for (const auto& d : engine.documents())
                if (d.title == name) { found = true; break; }
            if (!found) std::cerr << "Eval warning: unknown file '" << name
                                    << "' in query '" << q.query << "'\n";
        }

        if (!eval_queries.empty()) {
        EvalReport rep = evaluate(engine, eval_queries, 10);
            }
        {
            const size_t K = 10;
            EvalReport rep = evaluate(engine, eval_queries, K);

            std::cout << "\n========== Evaluation ==========\n\n";
            std::cout << "Queries:    " << rep.num_queries << "\n";
            std::cout << "Recall@" << rep.k << ":  " << rep.recall_at_k << "\n";
            std::cout << "MRR:        " << rep.mrr << "\n";
            std::cout << "================================\n\n";
        }
        // ----- end eval mode -----
        exit(-1);
    }

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    std::string input;


    


    while (true) {
        std::cout << "Enter something: ";
        std::getline(std::cin, input); 


        // Pass the user's input clearvariable into the function
        engine.print_query(input);
        std::cout << "You typed: " << input << "\n";
        
    }

}
