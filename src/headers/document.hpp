#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>

struct Document{
    size_t len;
    size_t id;
    std::unordered_map<std::string, size_t> term_freq; 
    Document(std::string& content, size_t doc_id);
    void print_doc() const;
};

class Engine{
public:
    std::vector<Document> docs;
    std::unordered_map<std::string, size_t> doc_freq; 

    double avg_doc_len;
    size_t total_docs;

    void add_doc(const std::string& text);
    void build_index(std::vector<std::string> corpus);
    void print_engine();

    size_t doc_frequency;

};