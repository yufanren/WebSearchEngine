//
// Created by yufan on 10/1/21.
//
#include "FileLoader.h"
#include "UrlTable.h"

//This is the main program that parse the source file
//into word lists, count the occurrence of word in each
//posting and write to temp files. When temp file size
//is reached trigger writing to disc.
void processSource(int mode) {
    std::ifstream infile(DEFAULT_PATH);
    bool inText = false;
    std::string content, url;
    unsigned int postIndex = 0;
    unsigned long long offset = 0;
    unsigned int temp_file_no = 0;
    int threadsAllowed = getNumThreadsAllowed(MAX_TEMP_ENTRIES_PER_FILE);
    std::vector<std::string> words;
    auto *tmpPosts = new std::vector<interimPosting>();
    tmpPosts->reserve(MAX_TEMP_ENTRIES_PER_FILE+MAX_TEMP_ENTRIES_PER_FILE/20);
    std::queue<pageData*> urlTable;
    std::vector<std::thread> workerList;
    struct pageData *page;
    std::mutex mutex;

    //start a thread for writing the urls table.
    auto urlWriter = std::thread(WriteURLs, &urlTable, &mutex, mode);

    //parsing source file.
    for (std::string line; getline(infile, line);) {

//        if (postIndex >= TEMP_FILE_LIMIT)
//            break;

        if (line.rfind("<TEXT>", 0) == 0) {
            inText = true;
            postIndex++;
            offset = infile.tellg();
        }
        else if (line.rfind("</TEXT>", 0) == 0) {
            inText = false;
//            std::cout << offset << std::endl;
            page = new pageData{postIndex, url, (unsigned int)((unsigned long long)infile.tellg() - offset - 8), offset};
            mutex.lock();
            urlTable.push(page);
            mutex.unlock();
            appendPostings(words, tmpPosts, postIndex);
            if (tmpPosts->size() >= MAX_TEMP_ENTRIES_PER_FILE) {

                waitThreads(workerList, threadsAllowed);

                workerList.push_back(std::thread(postTmp, tmpPosts, temp_file_no));

                tmpPosts = new std::vector<interimPosting>();
                tmpPosts->reserve(MAX_TEMP_ENTRIES_PER_FILE+MAX_TEMP_ENTRIES_PER_FILE/20);
                temp_file_no++;

            }
//            offset = 0;
        }
        else if (line.rfind("http", 0) == 0) {
            char* l = const_cast<char*>(line.c_str());
            url = std::strtok(l, " \t\v\f!");
        }
        else if (inText) {
            parseLine(line, words);
        }
    }
    if (!tmpPosts->empty())
        workerList.push_back(std::thread(postTmp, tmpPosts, temp_file_no));

    urlTable.push(nullptr);
    infile.close();
    urlWriter.join();
    for (auto& th: workerList) th.join();
}

//parse each line from source file
//at end of each document, triggers sorting and counting
//of words and adding to posting list.
void parseLine(std::string &line, std::vector<std::string> &words) {

    int start = 0, length = line.length();
//    offset += length;
    bool inWord = false, hasAlphabet = false;
    std::string piece;
    for (int i = 0; i < length; i++) {
        line[i] = std::tolower(line[i]);
        if (isalnum(line[i])) {
            if (!hasAlphabet && std::isalpha(line[i]))
                hasAlphabet = true;
            if (!inWord) {
                start = i;
                inWord = true;
            }
        }
        else {
            if (inWord) {
                inWord = false;
                if (hasAlphabet) {
                    piece = line.substr(start, i - start);
                    if (piece.length() <= LONGEST_WORD_LENGTH)
                        words.push_back(piece);
                }
                hasAlphabet = false;
            }
        }
    }
    if (inWord) {
        if (hasAlphabet) {
            piece = line.substr(start, length - start);
            if (piece.length() <= LONGEST_WORD_LENGTH)
                words.push_back(piece);
        }
    }
}

//Add postings from a doc into a vector.
void appendPostings(std::vector<std::string> &words, std::vector<interimPosting> *tmpPosts, unsigned int postIndex) {

    if (words.empty())
        return;
    std::sort(words.begin(), words.end());

    std::string current = words[0];
    unsigned int count = 0;
    struct interimPosting post;

    unsigned long size  = words.size();
    for (int i = 0; i < size; i++) {
        if (words[i] == current)
            count++;
        else {
            post = {current, postIndex, count};
            tmpPosts->push_back(post);
            current = words[i];
            count = 1;
        }
    }
    post = {current, postIndex, count};
    tmpPosts->push_back(post);
    words.clear();
}

//Sort and write postings to disc.
void postTmp(std::vector<interimPosting> *tmpPosts, int tempFileNum) {
    std::sort(tmpPosts->begin(), tmpPosts->end(), compareInterimPosting);

    std::string _fileName = "temp/temp_" + std::to_string(tempFileNum) + ".txt";
    std::ofstream tempFile(_fileName);

    struct interimPosting post;
    unsigned long size = tmpPosts->size();
    for (unsigned long i = 0; i < size; i++) {
        post = (*tmpPosts)[i];
        tempFile << post.word << " " << post.docId << " " << post.freq << std::endl;
    }
    tempFile.close();
    tmpPosts->clear();
    delete tmpPosts;
}

bool compareInterimPosting(const interimPosting &a, const interimPosting &b) {
    return std::tie(a.word, a.docId) < std::tie(b.word, b.docId);
}


void waitThreads(std::vector<std::thread> &threads, int numAllowed) {
    int pos = (int)(threads.size()) - numAllowed;
    if (pos >=0)
        threads[pos].join();
}

//Because some data is kept in memory before writing, it is necessary
//to limit the number of threads if individual intermediate files
//are set to be large.
int getNumThreadsAllowed(unsigned long entriesPerVector) {
    if (entriesPerVector > 65000000)
        return 1;
    else if (entriesPerVector > 50000000)
        return 2;
    else if (entriesPerVector > 40000000)
        return 3;
    else if (entriesPerVector > 30000000)
        return 4;
    else return 100;
}