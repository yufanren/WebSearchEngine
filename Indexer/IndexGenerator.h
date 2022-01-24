//
// Created by yufan on 10/1/21.
//

#ifndef RINDEXER_INDEXGENERATOR_H
#define RINDEXER_INDEXGENERATOR_H

#include <iostream>
#include "FileLoader.h"
#include <queue>
#include <mutex>
#include "Lexicon.h"
#include "Varbyte.h"

void MergeTempIndex();
void GenerateInvertedIndex(int mode);

class InvertedIndex {
private:
    int fileMode;
    std::string currentWord;
    std::vector<interimPosting*> singleWordEntries;
    unsigned long long fileCounter;
    FILE *fp_out;
    Vbyte *encoder;
    void writeList();
public:
    std::queue<LexiconTerm*> lexicon;
    std::mutex mutex;
    InvertedIndex(int mode, FILE *mergedPosts);
    ~InvertedIndex();
    void addPosting(interimPosting *post);
    void writeLastList();
    unsigned long long getCurrentPosition();
    unsigned long long getOutFilePosition();
};

#endif //RINDEXER_INDEXGENERATOR_H
