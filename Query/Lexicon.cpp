//
// Created by yufan on 11/1/21.
//

#include "Lexicon.h"

//Read the lexicon table from file into memory.
Lexicon::Lexicon(const std::string& lexicon) {

    termMap = new std::unordered_map<std::string, lexiconEntry>();
    std::ifstream infile(lexicon, std::ios::binary|std::ios::in);

    std::string term;
    unsigned int termLength, numDocs, listSize;
    unsigned long long offset;
    char* temp;

    if (!infile.is_open()) {
        std::cout << "Could not open url file!\n";
        return;
    }
    while (!infile.eof()) {

        infile.read((char*)&termLength, sizeof(unsigned int));
        temp = new char[termLength+1];
        infile.read(temp, termLength);
        temp[termLength] = '\0';
        term = temp;
        infile.read((char*)&numDocs, sizeof(unsigned int));
        infile.read((char*)&offset, sizeof(unsigned long long));
        infile.read((char*)&listSize, sizeof(unsigned int));
        delete [] temp;

        lexiconEntry entry = {
                numDocs,
                offset,
                listSize,
        };

        termMap->emplace(term, entry);
    }
    infile.close();
}

//Fetch the data from lexicon table given a term/
lexiconEntry Lexicon::getTerm(const std::string& term) {
    return (*termMap)[term];
}

Lexicon::~Lexicon() {
    termMap->clear();
    delete termMap;
}
