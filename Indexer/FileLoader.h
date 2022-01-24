//
// Created by yufan on 10/9/21.
//

#ifndef RINDEXER_FILELOADER_H
#define RINDEXER_FILELOADER_H

#include <vector>
#include <fstream>
#include <iostream>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include <fstab.h>
#include <tuple>
#include <thread>
#include "config.h"
#include "UrlTable.h"

typedef struct interimPosting {
    std::string word;
    unsigned int docId;
    unsigned int freq;
} interimPosting;

void parseLine(std::string &line, std::vector<std::string> &words);
void processSource(int mode);
void appendPostings(std::vector<std::string> &words, std::vector<interimPosting> *tmpPosts, unsigned int postIndex);
void postTmp(std::vector<interimPosting> *tmpPosts, int tempFileNum);
bool compareInterimPosting(const interimPosting &a, const interimPosting &b);
void writeUrlTable(std::vector<pageData> &urlTable);
void waitThreads(std::vector<std::thread> &threads, int numAllowed);
int getNumThreadsAllowed(unsigned long entriesPerVector);

#endif
