//
// Created by yufan on 10/1/21.
//

#include <cstdio>
#include <iostream>
#include "config.h"
#include "IndexGenerator.h"
#include <cstdlib>
#include <queue>
#include <thread>
#include "FileLoader.h"
#include "Lexicon.h"

//Scan sorted and merged postings and write to disc as inverted
//list and lexicon.
void GenerateInvertedIndex(int mode) {

    FILE *mergedPosts = fopen(MERGED_TEMP_INDEX_PATH, "r");
    if (!mergedPosts) {
        std::cout << "Failed to open merged posts file!\n";
    }
    char word[LONGEST_WORD_LENGTH];
    interimPosting *posting;
    unsigned int docId;
    unsigned int frequency;
    std::string currentWord;
    InvertedIndex invertedIndex(mode, mergedPosts);

    std::thread lexiconWriter = std::thread(WriteLexicon, &(invertedIndex.lexicon), &(invertedIndex.mutex), mode);
    while (fscanf(mergedPosts,  "%s %d %d\n", &word, &docId, &frequency) != EOF) {

        posting = new interimPosting{word, docId, frequency};
        invertedIndex.addPosting(posting);
    }

    invertedIndex.writeLastList();
    invertedIndex.lexicon.push(nullptr);
    lexiconWriter.join();
//    system(DELETE_MERGED_POSTINGS);
}

InvertedIndex::InvertedIndex(int mode, FILE *mergedPosts) {

    fileMode = mode;
    if (mode == FILE_MODE_ASCII)
        fp_out = fopen(INVERTED_INDEX_PATH, "w");
    else if (mode == FILE_MODE_BINARY)
        fp_out = fopen(INVERTED_INDEX_PATH, "wb");
    currentWord = "";
    encoder = new Vbyte();
    fileCounter = 0;
}

//Write inverted list to disc. If in ASCII mode, delimiters are added
//for separation and readability. If in Binary mode, there is no delimiters.
//doc ids are represented as difference of current and previous doc id.
//numbers are encoded with varbyte.
void InvertedIndex::writeList() {

    if (fileMode == FILE_MODE_ASCII) {
        fileCounter += fprintf(fp_out, "%u@", (unsigned int)(singleWordEntries.size()));
        for (auto& entry : singleWordEntries) {
            fileCounter += fprintf(fp_out, "%u_%u ", entry->docId, entry->freq);
            delete entry;
        }
        fileCounter += fprintf(fp_out, "\n");
    }
    else if (fileMode == FILE_MODE_BINARY) {
        unsigned int listSize = singleWordEntries.size();
        unsigned int numFullBlocks = listSize / INVERTED_LIST_ENTRIES_PER_BLOCK;
//        fileCounter += encoder->writeVbyte((uint64_t)INVERTED_LIST_ENTRIES_PER_BLOCK, fp_out);
        fileCounter += encoder->writeVbyte(listSize, fp_out);

        char *entriesBuffer = (char*)malloc(MAX_VBYTE_SIZE_UNSIGNED_INT * sizeof(char) * listSize * 2);
        char *docIdBuffer = (char*)malloc(MAX_VBYTE_SIZE_UNSIGNED_INT * sizeof(char) * (numFullBlocks+(listSize % INVERTED_LIST_ENTRIES_PER_BLOCK == 0 ? 0 : 1)));
        char *blockSizeBuffer = (char*)malloc(MAX_VBYTE_SIZE_UNSIGNED_INT * sizeof(char) * (numFullBlocks+(listSize % INVERTED_LIST_ENTRIES_PER_BLOCK == 0 ? 0 : 1)) * 2);
        char *currentEntryPtr = entriesBuffer, *currentDocIdPtr = docIdBuffer, *currentBlockSizePtr = blockSizeBuffer;

        unsigned int start, end, entrySteps, docIdSteps, blockSizeSteps, lastCompressedLength;
        unsigned int lastDoc = 0, compressedLength = 0, lastDocIdLength = 0, blockSizeLength = 0;

        for (unsigned int i = 0; i < numFullBlocks; i++) {
            start = i * INVERTED_LIST_ENTRIES_PER_BLOCK;
            end = (i + 1) * INVERTED_LIST_ENTRIES_PER_BLOCK;

            lastCompressedLength = compressedLength;
            for (unsigned int j = start; j < end; j++) {
                entrySteps = encoder->writeToBuffer(singleWordEntries[j]->docId - lastDoc, currentEntryPtr);
                lastDoc = singleWordEntries[j]->docId;
                compressedLength += entrySteps;
                currentEntryPtr += entrySteps;
            }
            blockSizeSteps = encoder->writeToBuffer( compressedLength - lastCompressedLength, currentBlockSizePtr);
            blockSizeLength += blockSizeSteps;
            currentBlockSizePtr += blockSizeSteps;

            lastCompressedLength = compressedLength;
            for (unsigned int j = start; j < end; j++) {
                entrySteps = encoder->writeToBuffer(singleWordEntries[j]->freq, currentEntryPtr);
                compressedLength += entrySteps;
                currentEntryPtr += entrySteps;
                delete singleWordEntries[j];
            }
            docIdSteps = encoder->writeToBuffer(lastDoc, currentDocIdPtr);
            lastDocIdLength += docIdSteps;
            currentDocIdPtr += docIdSteps;

            blockSizeSteps = encoder->writeToBuffer(compressedLength - lastCompressedLength, currentBlockSizePtr);
            blockSizeLength += blockSizeSteps;
            currentBlockSizePtr += blockSizeSteps;
        }
        lastCompressedLength = compressedLength;
        for (unsigned int i = numFullBlocks * INVERTED_LIST_ENTRIES_PER_BLOCK; i < listSize; i++) {
            entrySteps = encoder->writeToBuffer(singleWordEntries[i]->docId - lastDoc, currentEntryPtr);
            lastDoc = singleWordEntries[i]->docId;
            compressedLength += entrySteps;
            currentEntryPtr += entrySteps;
        }
        blockSizeSteps = encoder->writeToBuffer( compressedLength - lastCompressedLength, currentBlockSizePtr);
        blockSizeLength += blockSizeSteps;
        currentBlockSizePtr += blockSizeSteps;

        lastCompressedLength = compressedLength;
        for (unsigned int i = numFullBlocks * INVERTED_LIST_ENTRIES_PER_BLOCK; i < listSize; i++) {
            entrySteps = encoder->writeToBuffer(singleWordEntries[i]->freq, currentEntryPtr);
            compressedLength += entrySteps;
            currentEntryPtr += entrySteps;
            delete singleWordEntries[i];
        }
        docIdSteps = encoder->writeToBuffer(lastDoc, currentDocIdPtr);
        lastDocIdLength += docIdSteps;
        currentDocIdPtr += docIdSteps;

        blockSizeSteps = encoder->writeToBuffer(compressedLength - lastCompressedLength, currentBlockSizePtr);
        blockSizeLength += blockSizeSteps;
        currentBlockSizePtr += blockSizeSteps;

        entrySteps = fwrite(docIdBuffer, sizeof(char), lastDocIdLength, fp_out);
        if (entrySteps != lastDocIdLength) {
            std::cerr << "Didn't write intended length to file!\n";
            exit(1);
        }
        else
            fileCounter += entrySteps;
        entrySteps = fwrite(blockSizeBuffer, sizeof(char), blockSizeLength, fp_out);
        if (entrySteps != blockSizeLength) {
            std::cerr << "Didn't write intended length to file!\n";
            exit(1);
        }
        else
            fileCounter += entrySteps;
        entrySteps = fwrite(entriesBuffer, sizeof(char), compressedLength, fp_out);
        if (entrySteps != compressedLength) {
            std::cerr << "Didn't write intended length to file!\n";
            exit(1);
        }
        else
            fileCounter += entrySteps;

        free(entriesBuffer);
        free(docIdBuffer);
        free(blockSizeBuffer);
    }
    singleWordEntries.clear();
}

//Add a posting to a vector. If a new word is encountered trigger writing
//of the old word to disc.
void InvertedIndex::addPosting(interimPosting *post) {

    if (singleWordEntries.empty()) {
        currentWord = post->word;
    }
    else if (currentWord != post->word) {
        auto *next = new LexiconTerm;
        next->word = currentWord;
        next->numDocs = (unsigned int)(singleWordEntries.size());
        next->invertedIndexPtr = fileCounter;
        writeList();
        next->listSize = fileCounter - next->invertedIndexPtr;
        mutex.lock();
        lexicon.push(next);
        mutex.unlock();
        currentWord = post->word;
    }
    singleWordEntries.push_back(post);
}

//At the end writing the remaining word to inverted list.
void InvertedIndex::writeLastList() {
    if (!singleWordEntries.empty()) {
        auto *next = new LexiconTerm;
        next->word = currentWord;
        next->numDocs = (unsigned int)(singleWordEntries.size());
        next->invertedIndexPtr = fileCounter;
        writeList();
        next->listSize = fileCounter - next->invertedIndexPtr;
        mutex.lock();
        lexicon.push(next);
        mutex.unlock();
    }
}

InvertedIndex::~InvertedIndex() {
    fclose(fp_out);
    delete encoder;
}

//Call unix sort to merge sorted intermediate files
void MergeTempIndex() {
    system(MERGE_TEMP_FILES_COMMAND);
//    system(DELETE_TEMP_FILES);
}

unsigned long long InvertedIndex::getCurrentPosition() {
    return fileCounter;
}

unsigned long long InvertedIndex::getOutFilePosition() {
    return ftell(fp_out);
}


