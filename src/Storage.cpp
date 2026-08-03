#include "headers/Storage.hpp"


void Storage::save(const Engine& engine, const std::string& filename)
{
    std::ofstream out(filename, std::ios::binary);

    if (!out)
        throw std::runtime_error("Failed to open file.");

    const char magic[] = {'B', 'M', '2', '5'};
    constexpr uint32_t VERSION = 1;

    out.write(magic, sizeof(magic));
    out.write(reinterpret_cast<const char*>(&VERSION), sizeof(VERSION));

    const InvertedIndex& index = engine.index();
    const auto& docs = engine.documents();

    // total_len
    size_t total_len = index.total_length();
    out.write(reinterpret_cast<const char*>(&total_len), sizeof(total_len));

    // doc_count
    size_t doc_count = index.doc_len.size();
    out.write(reinterpret_cast<const char*>(&doc_count), sizeof(doc_count));

    // doc_len[]
    for (size_t len : index.doc_len)
    {
        out.write(reinterpret_cast<const char*>(&len), sizeof(len));
    }

    // document_count
    size_t document_count = docs.size();
    out.write(reinterpret_cast<const char*>(&document_count), sizeof(document_count));

    // document_names[]
    for (const Document& doc : docs)
    {
        size_t length = doc.title.size();

        out.write(reinterpret_cast<const char*>(&length), sizeof(length));
        out.write(doc.title.data(), length);
    }

    // term_count
    size_t term_count = index.index.size();
    out.write(reinterpret_cast<const char*>(&term_count), sizeof(term_count));

    // dictionary
    for (const auto& [term, postings] : index.index)
    {
        size_t term_length = term.size();

        out.write(reinterpret_cast<const char*>(&term_length), sizeof(term_length));
        out.write(term.data(), term_length);

        size_t posting_count = postings.size();
        out.write(reinterpret_cast<const char*>(&posting_count), sizeof(posting_count));

        for (const Posting& posting : postings)
        {
            out.write(reinterpret_cast<const char*>(&posting.doc_id), sizeof(posting.doc_id));
            out.write(reinterpret_cast<const char*>(&posting.term_freq), sizeof(posting.term_freq));
        }
    }
}


void Storage::load(Engine& engine, const std::string& filename)
{
    std::ifstream in(filename, std::ios::binary);

    if (!in)
        throw std::runtime_error("Failed to open file.");

    engine.clear();

    std::array<unsigned char, 4> magic{};
    uint32_t version;

    if (!in.read(reinterpret_cast<char*>(magic.data()), magic.size()))
        throw std::runtime_error("Failed to read magic bytes.");

    if (!in.read(reinterpret_cast<char*>(&version), sizeof(version)))
        throw std::runtime_error("Failed to read version.");

    constexpr std::array<unsigned char, 4> expected{'B', 'M', '2', '5'};

    if (magic != expected)
        throw std::runtime_error("Invalid index file.");

    if (version != 1)
        throw std::runtime_error("Unsupported index version.");

    auto& index = engine.index();
    auto& docs  = engine.documents();
    // total_len
    size_t total_len;
    in.read(reinterpret_cast<char*>(&total_len), sizeof(total_len));
    index.set_total_length(total_len);

    // doc_len
    size_t doc_count;
    in.read(reinterpret_cast<char*>(&doc_count), sizeof(doc_count));

    index.doc_len.resize(doc_count);

    for (size_t& len : index.doc_len)
    {
        in.read(reinterpret_cast<char*>(&len), sizeof(len));
    }

    // documents
    size_t document_count;
    in.read(reinterpret_cast<char*>(&document_count), sizeof(document_count));

docs.reserve(document_count);
for (size_t i = 0; i < document_count; ++i) {
    size_t string_length;
    in.read(reinterpret_cast<char*>(&string_length), sizeof(string_length));
    std::string title(string_length, '\0');
    in.read(title.data(), string_length);
    docs.emplace_back(std::vector<std::string_view>{}, i, std::move(title));
}

    // dictionary
    size_t term_count;
    in.read(reinterpret_cast<char*>(&term_count), sizeof(term_count));

    for (size_t i = 0; i < term_count; ++i)
    {
        size_t string_length;
        in.read(reinterpret_cast<char*>(&string_length), sizeof(string_length));

        std::string term(string_length, '\0');
        in.read(term.data(), string_length);

        size_t posting_count;
        in.read(reinterpret_cast<char*>(&posting_count), sizeof(posting_count));

        auto& postings = index.index[term];
        postings.resize(posting_count);

        for (Posting& posting : postings)
        {
            in.read(reinterpret_cast<char*>(&posting.doc_id), sizeof(posting.doc_id));
            in.read(reinterpret_cast<char*>(&posting.term_freq), sizeof(posting.term_freq));
        }
    }
}

 bool Storage::is_valid_index(const std::string& filename){
    std::ifstream in(filename, std::ios::binary);

    if (!in.is_open()){
        std::cerr << "Error: Could not open file.\n";
        return false;
    }



    const std::array<unsigned char,4> expected{'B','M','2','5'};
    std::array<unsigned char, 4> magic{};
    in.read(reinterpret_cast<char*>(magic.data()), magic.size());

    if (in.gcount() < 4){
        return false;
    }
    return magic == expected;
}
/*
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


    index.bin

├── magic
├── version
├── total_len
├── doc_count
├── doc_len[]
├── document_count
├── document_names[]
├── term_count
│
├── term
│   posting_count
│   posting...
│
├── term
│   posting_count
│   posting...
│
└── ...
}; */