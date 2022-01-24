//
// Created by yufan on 11/1/21.
//

#ifndef QUERY_INDEXITEM_H
#define QUERY_INDEXITEM_H
#include <cstdio>
#include <fstream>
#include "Varbyte.h"
#include "config.h"

class invertedIndexItem {
private:
public:
    unsigned int *blockEndDocId;
    unsigned int *blockOffsets;
    unsigned int **DocIds;
    unsigned int **freqs;
    std::string term;
    unsigned int size;
    unsigned int lastBlockSize;
    unsigned int numBlocks;
    unsigned long long dataStart;
    std::ifstream *fp_in;
    invertedIndexItem(std::ifstream &infile, std::string term, unsigned long long offset);
    ~invertedIndexItem();
    void unpackDocIdBlock(unsigned int pos) const;
    void unpackFrequencyBlock(unsigned int pos) const;
    void unpackAll() const;

    int getDocId(unsigned int pos);
    int getFrequency(unsigned int pos);
};


#endif //QUERY_INDEXITEM_H
