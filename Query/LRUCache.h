//
// Created by yufan on 11/5/21.
//

#ifndef QUERY_LRUCACHE_H
#define QUERY_LRUCACHE_H

#include <unordered_map>
#include <vector>
#include <list>
#include "config.h"
#include <set>

class LRUCache {
private:
    std::list<std::string> LRUQueue;
    std::unordered_map<std::string, std::pair<std::list<std::string>::iterator , std::string*>> Dict;
    int maxSize;
    long long totalStringBytes;
public:
    LRUCache();
    ~LRUCache();
    bool addEntry(std::string &key, std::string* entry);
    std::string* unsafeGetText(std::string &key);
    void unsafeRenewEntry(std::string &key);
    void unsafeAddEntry(std::basic_string<char> key, std::string* entry);
    bool contains(std::string &key);
    int getCacheSize();
};

std::string constructKey(const std::set<std::string>& terms, std::string mode);

#endif //QUERY_LRUCACHE_H
