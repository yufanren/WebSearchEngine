//
// Created by yufan on 11/2/21.
//

//Abstraction for a single query inside a query processer.
#include "Query.h"

Query::Query(std::string str, URLTable &urltable, Lexicon &lexicon, LRUCache &cache, bool mode) {

    this->mode = mode;
    termSet = parseTerms(str);
    termIndex = new std::vector<invertedIndexItem*>();
    infile = new std::ifstream(INVERTED_INDEX_PATH, std::ios::binary|std::ios::in);
    this->urltable = &urltable;
    this->lexicon = &lexicon;
    this->cache = &cache;

    lexiconEntry entry;
    for (const auto& w : *termSet) {
        entry = lexicon.getTerm(w);
        if (entry.listSize && entry.numDocs)
            termIndex->push_back(openList(*infile, w, entry.offset));
    }
    std::sort(termIndex->begin(), termIndex->end(),
              [](const invertedIndexItem* a, const invertedIndexItem* b) {
                  return a->size < b->size;
              });

}

Query::~Query() {
    for (const auto& ptr: *termIndex)
        closeList(ptr);
    delete termIndex;
    infile->close();
    delete infile;
    delete termSet;
//    delete result;   //Not deleted for cache.
}

//Given a list of doc ids and BM25 scores, fetch the url, snippet and other
//metadata for display
void Query::displayResult(std::vector<std::pair<unsigned int, double>> &list) {

    std::string url, text, snippet;
    auto *output = new std::string;
    auto stats = getStats(list);
    auto num = list.size();
    for (int i = 0; i < num; i++) {
        url = urltable->getURL(list[i].first).url;
        text = urltable->getText(list[i].first);
        snippet = getSnippet(text, *termIndex);
        output->append(url).append((*stats)[i])
                .append("BM25 Score: ").append(std::to_string(list[i].second))
                .append("\n").append(snippet).append("\n\n");
    }
    std::string metadata = "Inverted List length:\n";
    for (const auto &term : *termIndex) {
        auto termStr = term->term;
        auto size = term->size;
        metadata.append(termStr).append(":\t").append(std::to_string(size)).append("\n");
    }
    output->append(metadata);
    std::cout << *output;
    result = output;
    std::string token = mode ? "&" : "|";
    cache->unsafeAddEntry(constructKey(*termSet, token), result);
    delete stats;
}

//Fetch top docs which meet the requirements of query.
//Use the DAAT method.
//Used with AND semantic.
void Query::getConjunctiveResult() {

    if (getFromCache(constructKey(*termSet, "&")))
        return;

    unsigned int num = termIndex->size();
    if (num == 0)
        return;
    unsigned int did = 0, d, i;
    unsigned int maxDocId = urltable->getNumDocs();

    double totalDocs = urltable->getNumDocs();
    double avgDocLength = urltable->getAvgLength();
    auto numDocs = new double[num];
    double frequency;

    auto comp = [](const std::pair<unsigned int, double> &a, const std::pair<unsigned int, double> &b) {
        return a.second > b.second;
    };
    std::priority_queue<std::pair<unsigned int, double>, std::vector<std::pair<unsigned int, double>>,
            decltype(comp)> resultPQ(comp);

    for (int j = 0; j < num; j++) {
        numDocs[j] = ((*termIndex)[j])->size;
    }
    while (did <= maxDocId) {
        did = nextGEQ(*((*termIndex)[0]), did);

        for (i = 0; (i < num) && ((d= nextGEQ(*((*termIndex)[i]), did)) == did); i++);
        if (d > did)
            did = d;
        else {
            double docLength = urltable->getURL(did).size;
            double totalBM25 = 0;
            for (i = 0; i < num; i++) { //loop through each term for a did.
                frequency = getFreq(*((*termIndex)[i]), did);
                totalBM25 += getBM25Score(frequency, numDocs[i], totalDocs, docLength, avgDocLength);
            }
            resultPQ.emplace(std::pair<unsigned int, double>(did, totalBM25));
            if (resultPQ.size() > TOP_RESULT_RETURNED)
                resultPQ.pop();
            did++;
        }
    }
    std::stack<std::pair<unsigned int, double>> Stack;
    while (!resultPQ.empty()) {
        Stack.push(resultPQ.top());
        resultPQ.pop();
    }
    std::vector<std::pair<unsigned int, double>> resultList;
    while (!Stack.empty()) {
        resultList.push_back(Stack.top());
        Stack.pop();
    }
    displayResult(resultList);
    delete [] numDocs;
}


//Fetch top docs which meet the requirements of query.
//Use the TAAT method.
//Used with OR semantic.
void Query::getDisjunctiveResult() {

    if (getFromCache(constructKey(*termSet, "|")))
        return;

    unsigned int num = termIndex->size();
    unsigned int maxDocId = urltable->getNumDocs();
    double totalDocs = urltable->getNumDocs();
    double avgDocLength = urltable->getAvgLength();

    auto numDocs = new double[num];
    for (int i = 0; i < num; i++) {
        numDocs[i] = ((*termIndex)[i])->size;
    }

    auto comp = [](const std::pair<unsigned int, double> &a, const std::pair<unsigned int, double> &b) {
        return a.second > b.second;
    };
    std::priority_queue<std::pair<unsigned int, double>, std::vector<std::pair<unsigned int, double>>,
            decltype(comp)> resultPQ(comp);

    auto scoreArr = new double[maxDocId+1]();
    for (int i = 0; i < num; i++) {
        auto term = (*termIndex)[i];
        unsigned int termListSize = ((*termIndex)[i])->size;
        for (unsigned int j = 0; j < termListSize; j++) {
            double frequency = term->getFrequency(j);
            unsigned int docId = term->getDocId(j);
            double docLength = urltable->getURL(docId).size;
            double BM25 = getBM25Score(frequency, numDocs[i], totalDocs, docLength, avgDocLength);
            scoreArr[docId] += BM25;
        }
    }
    for (unsigned int i = 1; i <= maxDocId; i++) {
        if (scoreArr[i] != 0) {
            resultPQ.emplace(std::pair<unsigned int, double>(i, scoreArr[i]));
            if (resultPQ.size() > TOP_RESULT_RETURNED)
                resultPQ.pop();
        }
    }
    std::stack<std::pair<unsigned int, double>> Stack;
    while (!resultPQ.empty()) {
        Stack.push(resultPQ.top());
        resultPQ.pop();
    }
    std::vector<std::pair<unsigned int, double>> resultList;
    while (!Stack.empty()) {
        resultList.push_back(Stack.top());
        Stack.pop();
    }
    displayResult(resultList);
    delete [] numDocs;
    delete [] scoreArr;
}

//Fetch a query result from cache, if the result is already present.
bool Query::getFromCache(std::string str) {
    if (cache->contains(str)) {
        std::cout << *(cache->unsafeGetText(str));
        cache->unsafeRenewEntry(str);
        return true;
    }
    return false;
}

//Get the metastats for each top doc in the result
//Includes doc id and length, frequency for each search term inside the doc
//and BM25 of the doc for this query
std::vector<std::string>* Query::getStats(std::vector<std::pair<unsigned int, double>> &list) {

    auto num = termIndex->size();
    auto freqsList = new std::vector<std::string>();
    for (const auto &pair : list) {
        std::string temp = "\nDocId: ";
        auto urlentry = urltable->getURL(pair.first);
        temp.append(std::to_string(pair.first)).append(" Doc length: ").append(std::to_string(urlentry.size))
            .append(" bytes\nTerm Frequencies:");

        for (int i = 0; i < num; i++) {
            std::string term = (*termIndex)[i]->term;
            std::string frequency = std::to_string(getFreq(*((*termIndex)[i]), pair.first));
            temp.append(" ").append(term).append(": ").append(frequency);
        }
        temp.append("\n");
        freqsList->push_back(temp);
    }
    return freqsList;
}

//get next equal to or greater using binary search.
//Used to find docs ids which contains all query terms.
unsigned int nextGEQ(invertedIndexItem &pList, unsigned int docId) {
    unsigned int arrLength = pList.numBlocks;
    unsigned int* arr = pList.blockEndDocId;
    unsigned int l = 0, r = arrLength - 1;

    while (l <= r) {
        unsigned int m = l + (r - l) / 2;
        if (docId == arr[m])
            return docId;
        else if (docId > arr[m]) {
            l = m + 1;
        }
        else {
            if (m == 0 || docId > arr[m-1]) {
                arrLength = (m == pList.numBlocks - 1) ?  pList.lastBlockSize : DOCS_PER_BLOCK;
                if (pList.DocIds[m] == nullptr)
                    pList.unpackDocIdBlock(m);
                arr = pList.DocIds[m];
                l = 0, r = arrLength - 1;
                while (l <= r) {
                    m = l + (r - l) / 2;
                    if (docId == arr[m])
                        return docId;
                    else if (docId > arr[m])
                        l = m + 1;
                    else {
                        if (m == 0 || docId > arr[m-1])
                            return arr[m];
                        else
                            r = m - 1;
                    }
                }
                return MAX_DOC_ID;
            }
            else {
                r = m - 1;
            }
        }
    }
    return MAX_DOC_ID;
}

//First use binary search to find the position of doc id that match
//the target, then fetch frequency from the same position inside the
//frequency blocks.
unsigned int getFreq(invertedIndexItem &pList, unsigned int docId) {

    unsigned int arrLength = pList.numBlocks;
    unsigned int* arr = pList.blockEndDocId;
    int l = 0, r = arrLength - 1;
    unsigned int position;

    while (l <= r) {
        int m = l + (r - l) / 2;
        if (docId == arr[m]) {
            position = (m == pList.numBlocks - 1) ? DOCS_PER_BLOCK * m + pList.lastBlockSize - 1 :(m+1) * DOCS_PER_BLOCK - 1;
            return pList.getFrequency(position);
        }
        else if (docId > arr[m]) {
            l = m + 1;
        }
        else {
            if (m == 0 || docId > arr[m-1]) {
                arrLength = (m == pList.numBlocks - 1) ?  pList.lastBlockSize : DOCS_PER_BLOCK;
                if (pList.DocIds[m] == nullptr)
                    pList.unpackDocIdBlock(m);
                arr = pList.DocIds[m];
                l = 0, r = arrLength - 1;
                unsigned int blockNum = m;
                while (l <= r) {
                    m = l + (r - l) / 2;
                    if (docId == arr[m]) {
                        return pList.getFrequency(DOCS_PER_BLOCK * blockNum + m);
                    }
                    else if (docId > arr[m])
                        l = m + 1;
                    else
                        r = m - 1;
                }
                return 0;
            }
            else {
                r = m - 1;
            }
        }
    }
    return 0;
}

//open a lexicon entry for read from the inverted list
invertedIndexItem* openList(std::ifstream &input, std::string str, unsigned long long offset) {
    return new invertedIndexItem(input, str, offset);
}

//closes the inverted list entry.
void closeList(invertedIndexItem *item) {
    delete item;
}

//parse the query terms from user input.
std::set<std::string>* parseTerms(std::string &str) {
    auto result = new std::set<std::string>();
    int start = 0, length = str.length();
    bool inWord = false, hasAlphabet = false;
    std::string piece;
    for (unsigned int i = 0; i < length; i++) {
        str[i] = std::tolower(str[i]);
        if (isalnum(str[i])) {
            if (!hasAlphabet && std::isalpha(str[i])) {
                hasAlphabet = true;
            }
            if (!inWord) {
                start = i;
                inWord = true;
            }
        }
        else {
            if (inWord) {
                inWord = false;
                if (hasAlphabet) {
                    piece = str.substr(start, i-start);
                    if (piece.length() <= LONGEST_WORD_LENGTH)
                        result->insert(piece);
                }
                hasAlphabet = false;
            }
        }
    }
    if (inWord) {
        if (hasAlphabet) {
            piece = str.substr(start, length - start);
            if (piece.length() <= LONGEST_WORD_LENGTH)
                result->insert(piece);
        }
    }
    return result;
}