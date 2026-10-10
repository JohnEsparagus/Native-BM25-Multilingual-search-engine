I built a multilingual BM25 and hybrid retrieval search engine, and document indexer.
This project was inspired by my love for languages, currently learning Arabic and Chinese.
Eager to develop something that bridges the gap between cultures and language barriers, I decided to develop this.

Chinese, Arabic and English....

Metrics
- MRR
- @Recall3, @Recall10

Search
Paragraphs are packed up to passage sizechosen to fit the dense models 128 token input.
BM25 k1 = 1.2 b = 0.75
Hybrid is run on reciprocal rank fusion 
For scoring, relevance of the query is judged per book.

Warning!! Without a GPU testing time may be around 20 minutes to generate embeddings ( huge novels )....




Setup


Tech 
- Snowball Lib (Stemming for English and Arabic)
- cpp-jieba (Segmentation for chinese in cpp)
- Pybind (Using C++ as an API)


Our corpus
-
- Contains many books randomly selected.
- THe length is not standard, which can be bad for results. ( Some passages are long essays, some are fantasy novels)
- havent removed some english in the books
- 
The embeddings takes really long to load on my machine as i dont have a cpu.
BM25 is blazing fast, and really accurate


My findings
For 3 books
Language	Ranker	Recall@3	MRR
English	BM25 (C++)	0.819	0.862
English	Dense	0.862	0.768
English	Hybrid (RRF)	0.862	0.862
Arabic	BM25 (C++)	1.000	0.964
Arabic	Dense	0.643	0.548
Arabic	Hybrid (RRF)	1.000	0.958

BM25 triumphs:

Queequeg: 1.00 / 0.50 / 1.00. Dense put The Mysteries of Udolpho first.
Odysseus: 1.00 / 0.50 / 1.00. Dense put Pride and Prejudice first.
pawnbroker: 1.00 / 0.33 / 1.00. Dense put The Sailor's Word-Book first.
شيرلوك هولمز, واطسون, مورس, كيتس: 1.00 / 0.00 / 1.00. Dense put الفاروق عمر first for all four.
Dense beats:

castle: 0.33 / 1.00 / 0.33. BM25 put Chambers's Dictionary first, while dense put The Mysteries of Udolpho first.
الخلافة: 0.50 / 1.00 / 0.50. BM25 put إبراهيم الثاني first, a novel that is not about the caliphate.
Hybrid loses to both:

whale: 1.00 / 1.00 / 0.50. Fusion ranked The Sailor's Word-Book above Moby Dick.
All three lose:

smuggler: 0.00 / 0.00 / 0.00. The Sailor's Word-Book came out first because it had 15 instances against Carmen's 9 and is mentioned in the qrels only once.

Arabic scores really low for dense retrieval, it just chooses the biggest books.
Perhaps its due to a native stemming issue in cpp.

Im going to try different queries just to see, if they are less specific but describe the theme well, will we get what we want?


Results (passages, K=3 books)
Language	Query set	Queries	Ranker	Recall@3	MRR
English	names/terms	23	BM25 (C++)	0.819	0.862
Dense	0.862	0.768
Hybrid (RRF)	0.862	0.862
English	thematic	17	BM25 (C++)	0.765	0.637
Dense	1.000	0.912
Hybrid (RRF)	1.000	0.873
Chinese	names/terms	42	BM25 (C++)	0.857	0.829
Dense	0.583	0.361
Hybrid (RRF)	0.929	0.833
Chinese	thematic	15	BM25 (C++)	0.467	0.333
Dense	0.467	0.389
Hybrid (RRF)	0.600	0.444
Arabic	names/terms	28	BM25 (C++)	1.000	0.964
Dense	0.643	0.548
Hybrid (RRF)	1.000	0.958
Arabic	thematic	16	BM25 (C++)	0.875	0.844
Dense	0.812	0.708
Hybrid (RRF)	0.938	0.781

Hybrid has the best recall. This is expected. Its the best for both worlds on more thematic queries.
Arabic isnt doing as well in MRR as the other languages.


What the test queries returned
Query	Top result	Right?
a detective investigates a young woman followed by a man on a bicycle	Arabic Sherlock Holmes story	yes
the second caliph of Islam and his conquests	الفاروق عمر (Arabic)	yes
proofs for the existence of God	الطبيعة وما بعد الطبيعة (Arabic)	yes
鲸鱼 (whale, Chinese)	Moby Dick (English)	yes
whale	Moby Dick (English)	yes
狐仙和书生的爱情故事 (fox spirits and scholars)	二刻拍案惊奇; the expected 聊斋志异 was third	partly
الحوت الأبيض (the white whale, Arabic)	Pride and Prejudice	no
Sherlock Holmes	The Secret of Chimneys (English), not the Arabic Holmes story	no
The pattern matches your findings: descriptions cross

