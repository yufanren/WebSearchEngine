//
// Created by yufan on 11/1/21.
//

#ifndef QUERY_LEXICON_H
#define QUERY_LEXICON_H
#include <iostream>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include "config.h"

typedef struct lexiconEntry {
    unsigned int numDocs;
    unsigned long long offset;
    unsigned int listSize;
} lexiconEntry;

class Lexicon {
private:
    std::unordered_map<std::string, lexiconEntry> *termMap;
public:
    explicit Lexicon(const std::string& lexicon);
    ~Lexicon();
    lexiconEntry getTerm(const std::string& term);
};


#endif //QUERY_LEXICON_H
