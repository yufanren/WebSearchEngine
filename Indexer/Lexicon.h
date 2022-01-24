//
// Created by yufan on 10/12/21.
//

#ifndef RINDEXER_LEXICON_H
#define RINDEXER_LEXICON_H
#include <string>

typedef struct LexiconTerm {
    std::string word;
    unsigned int numDocs;
    unsigned long long invertedIndexPtr;
    unsigned int listSize;
} LexiconTerm;

void WriteLexicon(std::queue<LexiconTerm*> *lexiconTable, std::mutex *mutex, int mode);

#endif //RINDEXER_LEXICON_H
