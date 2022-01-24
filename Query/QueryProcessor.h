//
// Created by yufan on 11/6/21.
//

#ifndef QUERY_QUERYPROCESSOR_H
#define QUERY_QUERYPROCESSOR_H
#include "config.h"
#include "Query.h"
#include "LRUCache.h"
#include <chrono>

class QueryProcessor {
private:
    Lexicon *lexicon;
    URLTable *urltable;
    LRUCache *cache;
public:
    QueryProcessor();
    ~QueryProcessor();
    void Start();
};


#endif //QUERY_QUERYPROCESSOR_H
