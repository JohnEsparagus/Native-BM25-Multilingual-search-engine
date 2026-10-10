#include "headers/tokeniserarabic.hpp"
#include "headers/stemmer.h"
#include <unordered_set>

static const std::unordered_set<std::string> Arabic_stopwords = {
    "في", "من", "على", "إلى", "الى", "عن", "مع", "أن", "ان", "إن", "أو", "او",
    "و", "ثم", "ما", "لا", "لم", "لن", "قد", "كل", "هو", "هي", "هم",
    "هذا", "هذه", "ذلك", "تلك", "التي", "الذي", "الذين", "كان", "كانت"
}; 

static void strip_arabic_punctuation(std::string& text){
    static const std::string punctuation[] = {"،", "؛", "؟", "«", "»", "٪", "۔"};
    for (const auto& p : punctuation){
        for (size_t pos = text.find(p); pos != std::string::npos; pos = text.find(p, pos)){
            text.replace(pos, p.size(), " ");
        }
    }
}

std::vector<std::string> tokenise_arabic(const std::string& doc){
    // snowball stemmer deals wiht diacrtiics and everything difficult abt arabic stemming.
    Stemmer stemmer("arabic");
    std::string text = doc;
    strip_arabic_punctuation(text);

    std::vector<std::string> tokens;
    const std::string delims = " \t\n\r.,;:!?\"'()[]{}<>-=/\\_*";

    size_t curr = 0;
    while (curr < text.size()){
        curr = text.find_first_not_of(delims, curr);
        if (curr == std::string::npos){
            break;
        }

        size_t token_end = text.find_first_of(delims, curr);
        if (token_end == std::string::npos){
            token_end = text.size();
        }

        std::string token = text.substr(curr, token_end - curr);
        if (!Arabic_stopwords.count(token)){
            std::string stemmed = stemmer.stem(token);
            if (!stemmed.empty()){
                tokens.push_back(std::move(stemmed));
            }
        }
        curr = token_end;
    }
    return tokens;
}
