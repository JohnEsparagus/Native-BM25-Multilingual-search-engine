# Findings: BM25 vs dense vs hybrid retrieval in three languages

*This summary was written by an AI assistant from the evaluation output on 2026-10-10. The author's own notes are in `research.md`.*

## Summary

A from-scratch C++ BM25 engine was compared against a multilingual dense embedding model, and against a hybrid of the two, on English, Chinese and Arabic books split into passages.

- On queries that are **names or specific terms**, BM25 beat dense retrieval in all three languages.
- On **thematic queries** that describe a book without using its words, dense retrieval won clearly in English, narrowly in Chinese, and lost to BM25 in Arabic.
- **Hybrid** had the best or joint-best Recall@3 in every one of the six tests. It was never the worst on MRR, but it was the best on MRR in only two.

The test is small (8–9 books and 15–42 queries per set), so these are indications, not benchmarks.

## Results

Recall@3 and MRR over books. The best value in each group is in bold.

| Language | Query set | Queries | Ranker | Recall@3 | MRR |
|---|---|---|---|---|---|
| English | names/terms | 23 | BM25 (C++) | **0.862** | **0.906** |
| | | | Dense | **0.862** | 0.790 |
| | | | Hybrid | **0.862** | 0.884 |
| English | thematic | 17 | BM25 (C++) | 0.765 | 0.637 |
| | | | Dense | **1.000** | **0.912** |
| | | | Hybrid | **1.000** | 0.873 |
| Chinese | names/terms | 42 | BM25 (C++) | 0.857 | 0.829 |
| | | | Dense | 0.583 | 0.361 |
| | | | Hybrid | **0.929** | **0.833** |
| Chinese | thematic | 15 | BM25 (C++) | 0.467 | 0.333 |
| | | | Dense | 0.467 | 0.389 |
| | | | Hybrid | **0.600** | **0.444** |
| Arabic | names/terms | 28 | BM25 (C++) | **1.000** | **0.964** |
| | | | Dense | 0.643 | 0.548 |
| | | | Hybrid | **1.000** | 0.958 |
| Arabic | thematic | 16 | BM25 (C++) | 0.875 | **0.844** |
| | | | Dense | 0.812 | 0.708 |
| | | | Hybrid | **0.938** | 0.781 |

## Setup

| | English | Chinese | Arabic |
|---|---|---|---|
| Books | 9 | 9 | 8 |
| Corpus size | 10.7 MB | 6.6 MB | 4.5 MB |
| Passages | 25,667 | 15,431 | 7,706 |
| Passage size (characters) | 500 | 150 | 400 |

- **Passages:** paragraphs are packed up to the passage size; longer paragraphs are cut. The sizes were chosen to fit the dense model's 128-token input.
- **BM25:** the C++ engine in this repo, k1 = 1.2, b = 0.75. English and Arabic use Snowball stemming; Chinese uses cppjieba word segmentation.
- **Dense:** `paraphrase-multilingual-MiniLM-L12-v2` (sentence-transformers), cosine similarity, on CPU.
- **Hybrid:** reciprocal rank fusion of the top 100 passages from each ranker, constant 60.
- **Relevance:** judged per book. A book ranks where its best passage ranks.
- **Metrics:** Recall@3, and MRR as the reciprocal rank of the first relevant book in the top 3.
- **Queries:** `eval/<language>.tsv` (names and terms) and `eval/<language>_thematic.tsv`.

Reproduce with `python src/python/test.py <english|chinese|arabic>`.

## Why the evaluation changed

The first version indexed each whole book as one document and measured Recall@10 and MRR on English:

| Setup | Ranker | Recall | MRR |
|---|---|---|---|
| Whole books, K=10 | BM25 | 0.913 | 0.957 |
| Whole books, K=10 | Dense | 1.000 | 0.603 |
| Passages, K=3 | BM25 | 0.862 | 0.906 |
| Passages, K=3 | Dense | 0.862 | 0.790 |

Two things were wrong with the first version, both noted in `research.md`:

- With 9 documents and K=10, every document that matched at all was returned, so Recall@10 said almost nothing.
- The dense model reads only 128 tokens, so it was ranking each book by its first few sentences.

Splitting books into passages fixed both. The two rows are not otherwise comparable: one query in `eval/english.tsv` was also corrected between them (`Herzoguvia` to `Herzoslovakia`, the spelling used in the book).

## Example queries

Reciprocal rank for BM25 / Dense / Hybrid.

| Query | Language | BM25 | Dense | Hybrid | What happened |
|---|---|---|---|---|---|
| `Queequeg` | English | 1.00 | 0.50 | 1.00 | Dense ranked The Mysteries of Udolpho first |
| `pawnbroker` | English | 1.00 | 0.33 | 1.00 | Dense ranked The Sailor's Word-Book first |
| `شيرلوك هولمز` | Arabic | 1.00 | 0.00 | 1.00 | Dense ranked a biography of Umar first |
| `西门庆` | Chinese | 1.00 | 0.33 | 1.00 | Dense ranked 闲情偶寄 first |
| `castle` | English | 0.33 | 1.00 | 0.33 | BM25 ranked Chambers's Dictionary first |
| `whale` | English | 1.00 | 1.00 | 0.50 | Fusion promoted The Sailor's Word-Book above Moby Dick |
| `smuggler` | English | 0.00 | 0.00 | 0.00 | All three ranked The Sailor's Word-Book first; only Carmen is marked relevant |

## Observations

- **Dense retrieval struggles with rare names.** In Arabic, 11 of the 28 name queries scored 0.00 or 0.33, almost all single proper names. In Chinese, the same book (`闲情偶寄`) was the top dense answer for nearly every name query.
- **Large and reference books attract results.** The two English reference books make up 10,446 of the 25,667 passages and took first place on several one-word queries. In Arabic, the largest book (3,636 of 7,706 passages) was the top dense answer for 7 of the failed name queries.
- **Hybrid protects against the weaker ranker.** It stayed close to BM25 where dense failed and close to dense where BM25 failed, at the cost of not always matching the better one.
- **Chinese BM25 returned no results for five name queries** (`庞春梅`, `王维`, `李商隐`, `李渔`, `慎独`). The cause has not been investigated; the name may be absent from the text or segmented differently in the query and the passage.

## Limitations

- The corpus is small: 8 or 9 books per language.
- Queries and relevance labels were written by hand, the thematic ones and all the Arabic ones by an AI assistant. They have not been independently checked.
- The thematic sets have 15–17 queries, so a single query moves MRR by about 0.06.
- Relevance is per book. A passage from the right book counts as correct even when that passage is not about the query.
- One dense model was tested, with no tuning of k1, b, passage size or the fusion constants.
- Two Chinese files begin with English Project Gutenberg licence text, so some Chinese passages are English boilerplate.
- Passage sizes differ by language, so scores are not directly comparable across languages.
