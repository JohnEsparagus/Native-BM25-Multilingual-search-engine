#include "headers/tokeniserchinese.hpp"

std::vector<std::string> tokenise_chinese(const std::string& doc){
    static cppjieba::Jieba jieba(
        "third_party/cppjieba/dict/jieba.dict.utf8",
        "third_party/cppjieba/dict/hmm_model.utf8",
        "third_party/cppjieba/dict/user.dict.utf8",
        "third_party/cppjieba/dict/idf.utf8",
        "third_party/cppjieba/dict/stop_words.utf8"
    );

    //input doc inside....
    std::vector<std::string> tokens;
    

    jieba.Cut(doc, tokens, true);
    return tokens;

}