Warning: You are sending unauthenticated requests to the HF Hub. Please set a HF_TOKEN to enable higher rate limits and faster downloads.
Loading weights: 100%|█████████████████| 199/199 [00:00<00:00, 5569.86it/s]
9 docs, 23 queries, K=10
BM25 (C++)     Recall@10: 0.913   MRR: 0.957
Dense (naive)  Recall@10: 1.000   MRR: 0.603

Ok, so the recall values dont seem to be too useful here. because I checked further and it seems that there were only around
10 values returned.... they both seem to be working in recall10
However on the MRR, it seems naive dense search is really poor. This was rlly odd so I looked further into it.
I realised my documents are ENTIRE BOOKS. I need to incorporate some smart chunking strategies to feed into the dense search.
Because What i did was i used the first extract of the book which isnt really good. I will try to improve this.
