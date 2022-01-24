//
// Created by yufan on 11/1/21.
//

#include "IndexItem.h"

#include <utility>

//This is the abstraction for a term entry inside the inverted list.
//Size for each data block is accumulated to give the offset of each block.
invertedIndexItem::invertedIndexItem(std::ifstream &infile, std::string term, unsigned long long offset) {
    infile.seekg(offset);
    this->term = std::move(term);
    size = readVarbyte(infile);
    unsigned int numFullBlocks = size / DOCS_PER_BLOCK;
    lastBlockSize = (size % DOCS_PER_BLOCK == 0) ? DOCS_PER_BLOCK : size % DOCS_PER_BLOCK;
    numBlocks = numFullBlocks + (lastBlockSize == DOCS_PER_BLOCK ? 0 : 1);

    blockEndDocId = new unsigned int[numBlocks];
    blockOffsets = new unsigned int[numBlocks*2];
    DocIds = new unsigned int*[numBlocks]();
    freqs = new unsigned int*[numBlocks]();

    for (unsigned int i = 0; i < numBlocks; i++) {
        blockEndDocId[i] = readVarbyte(infile);
    }

    unsigned int blockOffset = 0;
    for (unsigned int i = 0; i < numBlocks*2; i++) {
        blockOffsets[i] = blockOffset;
        blockOffset += readVarbyte(infile);
    }
    dataStart = infile.tellg();
    fp_in = &infile;
}

invertedIndexItem::~invertedIndexItem() {
    delete []blockEndDocId;
    delete []blockOffsets;
    for (int i = 0; i < numBlocks; i++) {
        delete DocIds[i];
        delete freqs[i];
    }
    delete []DocIds;
    delete []freqs;
}

//Decompress a block of doc ids
void invertedIndexItem::unpackDocIdBlock(unsigned int pos) const {
    if (DocIds[pos] != nullptr)
        return;
    unsigned int arrSize = (pos == numBlocks - 1) ? lastBlockSize : DOCS_PER_BLOCK;
    DocIds[pos] = new unsigned int[arrSize];
    unsigned int lastDocId = (pos == 0) ? 0 : blockEndDocId[pos-1];
    fp_in->seekg(dataStart + blockOffsets[pos*2]);
    for (unsigned int i = 0; i < arrSize; i++) {
        lastDocId += readVarbyte(*fp_in);
        DocIds[pos][i] = lastDocId;
    }
}

//Decompress a block of terms frequencies
void invertedIndexItem::unpackFrequencyBlock(unsigned int pos) const {
    if (freqs[pos] != nullptr)
        return;
    unsigned int arrSize = (pos == numBlocks - 1) ? lastBlockSize : DOCS_PER_BLOCK;
    freqs[pos] = new unsigned int[arrSize];
    fp_in->seekg(dataStart + blockOffsets[pos*2+1]);
    for (unsigned int i = 0; i < arrSize; i++) {
        freqs[pos][i] = readVarbyte(*fp_in);
    }
}

//Decompress all doc id and frequency blocks
void invertedIndexItem::unpackAll() const {

    for (unsigned int i = 0; i < numBlocks; i++) {
        unpackDocIdBlock(i);
        unpackFrequencyBlock(i);
    }
}

//get doc id from a specific location inside the doc id array
int invertedIndexItem::getDocId(unsigned int pos) {
    if (DocIds[pos/DOCS_PER_BLOCK] == nullptr)
        unpackDocIdBlock(pos/DOCS_PER_BLOCK);
    return (int)(DocIds[pos/DOCS_PER_BLOCK][pos%DOCS_PER_BLOCK]);
}

//get frequency from a specific location inside the frequencies array
int invertedIndexItem::getFrequency(unsigned int pos) {
    if (freqs[pos/DOCS_PER_BLOCK] == nullptr)
        unpackFrequencyBlock(pos/DOCS_PER_BLOCK);
    return (int)(freqs[pos/DOCS_PER_BLOCK][pos%DOCS_PER_BLOCK]);
}