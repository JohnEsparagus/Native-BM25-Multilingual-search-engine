#pragma once

#include <libstemmer.h>
#include <string>

class Stemmer{
public:
    Stemmer(){
        stemmer = sb_stemmer_new("english", "UTF_8");
    }

    ~Stemmer(){
        sb_stemmer_delete(stemmer);
    }

    std::string stem(const std::string& word){
        const sb_symbol* s = sb_stemmer_stem(
            stemmer, 
            reinterpret_cast<const sb_symbol*>(word.data()),
            word.size());
        return std::string(reinterpret_cast<const char*>(s));
    }

private:
    sb_stemmer* stemmer;
};