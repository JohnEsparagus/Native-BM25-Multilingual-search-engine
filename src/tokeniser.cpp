#include "headers/tokeniser.hpp"

void fast_lower(std::string& doc){
    for (char &c : doc){
        c |= (c >= 'A' && c <= 'Z') <<5;
    }
}


std::vector<std::string_view> tokenise( std::string& doc){
    fast_lower(doc);
    std::vector<std::string_view> tokens;
    std::string_view view(doc); //avoid copy and peer itno doc

    const std::string_view delims =  " \t\n\r.,;:!?\"'()[]{}<>-=/\\_*";
    //skip leading
    size_t curr = 0;
    while (curr < view.size()){

        curr = view.find_first_not_of(delims,curr);
        if (curr == std::string_view::npos){
            break;
        }

        size_t token_end = view.find_first_of(delims,curr);
        if (token_end==std::string_view::npos) {
            tokens.push_back(view.substr(curr));
            break;
        }

        std::string_view token = view.substr(curr, token_end - curr);
  
        if (!token.empty()){
            tokens.push_back(token);
        }
        curr = token_end;   
    }
    return tokens;
}

