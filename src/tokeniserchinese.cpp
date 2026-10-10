#include "headers/tokeniserchinese.hpp"
#include <cstdlib>
#include <filesystem>
#include <stdexcept>

#ifndef JIEBA_DICT_DIR
#define JIEBA_DICT_DIR "third_party/cppjieba/dict"
#endif

static cppjieba::Jieba& get_jieba() {
    // JIEBA_DICT_DIR in the environment wins, so the binary still works after being moved
    const char* env_dir = std::getenv("JIEBA_DICT_DIR");
    const std::string dir = env_dir ? env_dir : JIEBA_DICT_DIR;
    if (!std::filesystem::exists(dir + "/jieba.dict.utf8"))
        throw std::runtime_error("cppjieba dictionary not found in: " + dir);

    static cppjieba::Jieba jieba(
        dir + "/jieba.dict.utf8",
        dir + "/hmm_model.utf8",
        dir + "/user.dict.utf8",
        dir + "/idf.utf8",
        dir + "/stop_words.utf8");
    return jieba;
}

std::vector<std::string> tokenise_chinese(const std::string& doc) {
    std::vector<std::string> tokens;
    get_jieba().Cut(doc, tokens, true);
    return tokens;
}