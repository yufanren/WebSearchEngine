//
// Created by yufan on 10/12/21.
//
#include <queue>
#include <mutex>
#include "unistd.h"
#include "config.h"
#include "Lexicon.h"

//write data to lexicon table.
void WriteLexicon(std::queue<LexiconTerm*> *lexiconTable, std::mutex *mutex, int mode) {

    if(mode == FILE_MODE_ASCII) {
        FILE *fp = fopen(LEXICON_TABLE_PATH, "w");
        while (true) {
            if (lexiconTable->empty()) usleep(100);
            else {
                LexiconTerm *term = lexiconTable->front();
                mutex->lock();
                lexiconTable->pop();
                mutex->unlock();
                if (term == nullptr) {
                    fclose(fp);
                    break;
                }
                fprintf(fp, "%s %u %llu %u\n", term->word.data(), term->numDocs, term->invertedIndexPtr, term->listSize);
                delete term;
            }
        }
    }
    else if (mode == FILE_MODE_BINARY) {

        FILE *fp = fopen(LEXICON_TABLE_PATH, "wb");
        while (true) {
            if (lexiconTable->empty()) usleep(100);
            else {
                LexiconTerm *term = lexiconTable->front();
                mutex->lock();
                lexiconTable->pop();
                mutex->unlock();
                if (term == nullptr) {
                    fclose(fp);
                    break;
                }
                unsigned int wordLength = term->word.length();
                fwrite(&wordLength, sizeof(unsigned int), 1, fp);
                fwrite(term->word.data(), sizeof(char), wordLength, fp);
                fwrite(&term->numDocs, sizeof(unsigned int), 1, fp);
                fwrite(&term->invertedIndexPtr, sizeof(unsigned long long), 1, fp);
                fwrite(&term->listSize, sizeof(unsigned int), 1, fp);
                delete term;
            }
        }
    }
}
