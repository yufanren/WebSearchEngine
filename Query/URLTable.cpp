//
// Created by yufan on 11/1/21.
//

#include "URLTable.h"

//Reads the url table from file into main memory.
//Count the number of docs and calculate average length of the docs.
URLTable::URLTable(const std::string& urlTable) {

    urlMap = new std::unordered_map<unsigned int, urlEntry>();
    repository = REPOSITORY_PATH;
    std::ifstream infile(urlTable, std::ios::binary|std::ios::in);

    unsigned int docId, length, size;
    std::string url;
    unsigned long long offset, totalLength = 0;
    char* temp;
    numDocs = 0;

    if (!infile.is_open()) {
        std::cout << "Could not open url file!\n";
        return;
    }
    while (!infile.eof()) {

        infile.read((char*)&docId, sizeof(unsigned int));
        infile.read((char*)&length, sizeof(unsigned int));
        temp = new char[length+1];
        infile.read(temp, length);
        temp[length] = '\0';
        url = temp;
        infile.read((char*)&size, sizeof(unsigned int));
        infile.read((char*)&offset, sizeof(unsigned long long));
        delete [] temp;

        urlEntry entry = {
            url,
            size,
            offset,
        };

        urlMap->emplace(docId, entry);
        numDocs++;
        totalLength += size;
    }
    avgLength = totalLength / numDocs;
    infile.close();
}

//Given a doc id, find the records associated with it.
urlEntry URLTable::getURL(unsigned int docId) {
    return (*urlMap)[docId];
}

//Given a doc id, fetch the whole text of the doc from the original date file
//Text offset and size were pre-recorded inside the url table.
std::string URLTable::getText(unsigned int docId) {
    urlEntry entry = getURL(docId);
    FILE *urls = std::fopen(repository.data(), "r");
    fseek(urls, entry.offset, SEEK_SET);
    char* buffer = (char*) malloc(sizeof(char)*entry.size);
    unsigned int result = std::fread(buffer, sizeof(char), entry.size, urls);
    if (result != entry.size)
        std::cout << "Didn't read expected number of chars!\n";
    return convertToString(buffer, result);
}

URLTable::~URLTable() {
    urlMap->clear();
    delete urlMap;
}

unsigned int URLTable::getAvgLength() {
    return avgLength;
}

unsigned int URLTable::getNumDocs() {
    return numDocs;
}

//convert a char array to std::string.
std::string convertToString(char* a, unsigned int size)
{
    int i;
    std::string s;
    for (i = 0; i < size; i++) {
        s += a[i];
    }
    return s;
}