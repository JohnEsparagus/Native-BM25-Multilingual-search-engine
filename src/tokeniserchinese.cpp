#include "/headers/tokeniserchinese.hpp"

int main(){
    cppjieba::Jieba jieba(
        "cppjieba/dict/jieba.dict.utf8",
        "cppjieba/dict/hmm_model.utf8",
        "cppjieba/dict/user.dict.utf8",
        "cppjieba/dict/idf.utf8",
        "cppjieba/dict/stop_words.utf8"
    );

}