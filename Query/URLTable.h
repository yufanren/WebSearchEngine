//
// Created by yufan on 11/1/21.
//

#ifndef QUERY_URLTABLE_H
#define QUERY_URLTABLE_H
#include <iostream>
#include <cstdio>
#include <fstream>
#include <unordered_map>
#include "config.h"

typedef struct urlEntry {
    std::string url;
    unsigned int size;
    unsigned long long offset;
} urlEntry;

class URLTable {
private:
    std::unordered_map<unsigned int, urlEntry> *urlMap;
    std::string repository;
    unsigned int avgLength;
    unsigned int numDocs;
public:
    explicit URLTable(const std::string& urlTable);
    ~URLTable();
    urlEntry getURL(unsigned int docId);
    std::string getText(unsigned int docId);
    unsigned int getAvgLength();
    unsigned int getNumDocs();
};

std::string convertToString(char* a, unsigned int size);

#endif //QUERY_URLTABLE_H
