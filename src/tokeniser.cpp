#include "headers/tokeniser.hpp"

std::string fast_lower(std::string& doc){
    std::string out = doc;
    for (char &c : out){
        if (c >= 'A' && c <= 'Z') c+=32;
    }
    return out;
}


std::vector<std::string> tokenise(std::string& doc){
    std::string lowered = fast_lower(doc);
    std::vector<std::string> tokens;

    const std::string delims =  " \t\n\r.,;:!?\"'()[]{}<>-=/\\_*";
    //skip leading
    size_t curr = 0;
    while (curr < lowered.size()){

        curr = lowered.find_first_not_of(delims,curr);
        if (curr == std::string::npos){
            break;
        }

        size_t token_end = lowered.find_first_of(delims,curr);
        if (token_end==std::string::npos) {
            tokens.push_back(lowered.substr(curr));
            break;
        }

        std::string token = lowered.substr(curr, token_end - curr);
  
        if (!token.empty()){
            tokens.push_back(token);
        }
        curr = token_end;   
    }
    return tokens;
}

