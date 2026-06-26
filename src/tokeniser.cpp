#include "headers/tokeniser.hpp"

void fast_lower(std::string& doc){
    for (char &c : doc){
        c |= (c >= 'A' && c <= 'Z') <<5;
    }
}


std::vector<std::string_view> tokenise(std::string& doc){
    fast_lower(doc);
    std::vector<std::string_view> tokens;
    std::string_view view(doc); //avoid copy and peer itno doc

    //skip leading
    size_t curr = 0;
    while (curr < view.size()){

        curr = view.find_first_not_of(" \t\n\r",curr);
        if (curr == std::string_view::npos){
            break;
        }

        size_t token_end = view.find_first_of((" \t\n\r"),curr);
        if (token_end==std::string_view::npos) {
            tokens.push_back(view.substr(curr));
            break;
        }

        std::string_view token = view.substr(curr, token_end - curr);
        while (!token.empty() && ispunct(token.front())) token.remove_prefix(1);
        while (!token.empty() && ispunct(token.back())) token.remove_suffix(1);

        if (!token.empty()){
            tokens.push_back(token);
        }
        curr = token_end;   
    }
    return tokens;
}

