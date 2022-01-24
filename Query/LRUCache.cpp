//
// Created by yufan on 11/5/21.
//

#include "LRUCache.h"

//Least recently used implementation of query result cache.

LRUCache::LRUCache() {
    maxSize = CACHE_MAX_SIZE;
    totalStringBytes = 0;
}

LRUCache::~LRUCache() {
    for (const auto &element : Dict) {
        delete &(element.second.second);
    }
}

//Called when a hit is found in cache.
//place the hit term at last place to erase.
void LRUCache::unsafeRenewEntry(std::string &key) {
    //use this function only if entry already in cache.
    LRUQueue.erase(Dict[key].first);
    LRUQueue.push_front(key);
    Dict[key] = {LRUQueue.begin(), Dict[key].second};
}

//Called when query entry is not found in cache.
//Must check before calling.
void LRUCache::unsafeAddEntry(std::basic_string<char> key, std::string* entry) {
    //use this function only if there made sure the entry is not in cache.
    while (getCacheSize() >= maxSize) {
        auto last = LRUQueue.back();
        LRUQueue.pop_back();
        totalStringBytes -= (Dict[last].second->size() + 2 * last.size());
        delete Dict[last].second;
        Dict.erase(last);
    }
    LRUQueue.push_front(key);
    Dict[key] = {LRUQueue.begin(), entry};
    totalStringBytes += entry->size() + 2 * key.size();
}

bool LRUCache::contains(std::string &key) {
    return Dict.find(key) != Dict.end();
}

//Add a entry to the cache.
bool LRUCache::addEntry(std::string &key, std::string* entry) {

    bool added;
    if (Dict.find(key) == Dict.end()) {
        if (getCacheSize() >= maxSize) {
            auto last = LRUQueue.back();
            LRUQueue.pop_back();
            totalStringBytes -= (Dict[last].second->size() + 2 * last.size());
            delete Dict[last].second;
            Dict.erase(last);
        }
        added = true;
    }
    else {
        LRUQueue.erase(Dict[key].first);
        added = false;
    }
    LRUQueue.push_front(key);
    Dict[key] = {LRUQueue.begin(), entry};
    totalStringBytes += entry->size() + 2 * key.size();
    return added;
}

//given a term, fetch result corresponding to the term.
//Musy check if term is present in cache before calling.
std::string* LRUCache::unsafeGetText(std::string &key) {
    //Make sure key is already in cache!
    if (this->contains(key)) {
        return Dict[key].second;
    }
    return nullptr;
}

//Estimates the size of current cache in MB.
int LRUCache::getCacheSize() {
    auto numItems = LRUQueue.size();
    auto stringSize = numItems * sizeof(std::string) + totalStringBytes; //cached strings on heap.

    auto listSize = sizeof(std::list<std::string>) + numItems * (sizeof(std::string));
    auto dictSize = sizeof(std::unordered_map<std::string, std::pair<std::list<std::string>::iterator , std::string*>>)
            + Dict.max_bucket_count() * (sizeof(std::string) + sizeof(std::pair<std::list<std::string>::iterator, std::string*>)
            + sizeof(std::list<std::string>::iterator) + sizeof(std::string*));
    return (int)((stringSize + listSize + dictSize) / 1048576);
}

//Construct a string used for caching
//combines already sorted terms, appends a special char to indicate conjunctive/disjunctive
std::string constructKey(const std::set<std::string>& terms, std::string mode) {

    for (const auto &term : terms) {
        mode.append(" ").append(term);
    }
    return mode;
}