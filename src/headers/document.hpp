#pragma once
#include <unordered_map>
#include <string>

class Document{
private:
    
public:
    std::unordered_map<std::string, size_t> term_freq; 
    Document(const std::string& document);

    size_t len;
};


class Engine{
public:
    double avg_doc_len;
    size_t total_docs;
    size_t doc_frequency;

};