#pragma once
#include <unordered_map>
#include <string>
#include <vector>
#include <iostream>
#include <cmath>
#include <filesystem>
#include <fstream>
struct Posting{
    size_t doc_id;
    size_t term_freq;
};

struct Document{
    size_t id;
    size_t len;
    std::unordered_map<std::string, size_t> term_freq;

    Document(const std::vector<std::string_view>& tokens, size_t doc_id);
    void print_doc() const;
};

struct InvertedIndex{
    std::unordered_map<std::string, std::vector<Posting>> index;

    std::vector<size_t> doc_len; //0 -> N
    size_t total_docs() const {return doc_len.size();}
    void clear();
    
    double get_avg_doc_len() const {
        if (doc_len.empty())
            return 0.0;

        return static_cast<double>(total_len) / doc_len.size();
    }
    void add_doc(const Document& doc);
    size_t get_doc_freq(const std::string& term) const;
    const std::vector<Posting>* get_postings(const std::string& term) const;

    private:
    size_t total_len = 0;

};

struct Scorer{
    explicit Scorer(const InvertedIndex& index);

    double score_term(const std::string& term, size_t term_freq_in_doc, size_t doc_len) const;
    private:
    const InvertedIndex& index;
};




class Engine{
public:


    size_t total_docs() const { return docs_.size();}
    double average_doc_len()const {return index_.get_avg_doc_len();}

    void add_doc(const std::string& text);
    void build_index(std::vector<std::string>& corpus);
    void print_engine();

    void clear();

    void print_query(const std::string &query_text) const;

    std::vector<double> query(const std::string& query_text) const;



    private:
    std::vector<Document> docs_;
    InvertedIndex index_; 


};


class TextLoader{
public:
    void load_codex(std::filesystem::path& path, Engine& engine);

};