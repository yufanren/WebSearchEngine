//
// Created by yufan on 11/2/21.
//

#ifndef QUERY_QUERY_H
#define QUERY_QUERY_H
#include <iostream>
#include <set>
#include <stack>
#include <queue>
#include <algorithm>
#include "Lexicon.h"
#include "URLTable.h"
#include "IndexItem.h"
#include "BM25.h"
#include "Snippet.h"
#include "LRUCache.h"

class Query {
private:
    bool mode;
    std::vector<invertedIndexItem*> *termIndex;
    std::ifstream *infile;
    URLTable *urltable;
    Lexicon *lexicon;
    LRUCache *cache;
    std::set<std::string> *termSet;
    bool getFromCache(std::string str);
    void displayResult(std::vector<std::pair<unsigned int, double>> &list);
    std::vector<std::string>* getStats(std::vector<std::pair<unsigned int, double>> &list);
public:
    Query(std::string str, URLTable &urltable, Lexicon &lexicon, LRUCache &cache, bool mode);
    ~Query();
    void getConjunctiveResult();
    void getDisjunctiveResult();
    std::string *result;
};


invertedIndexItem* openList(std::ifstream &input, std::string str, unsigned long long offset);
void closeList(invertedIndexItem *item);
unsigned int nextGEQ(invertedIndexItem &pList, unsigned int docId);
unsigned int getFreq(invertedIndexItem &pList, unsigned int docId);

std::set<std::string>* parseTerms(std::string &str);


#endif //QUERY_QUERY_H
