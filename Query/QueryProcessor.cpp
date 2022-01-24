//
// Created by yufan on 11/6/21.
//

#include "QueryProcessor.h"

//This is the main program for getting input from user and drives
//the query process.
void QueryProcessor::Start() {
    std::string query, mode;
    while (std::cout<<"\nPlease enter query: \n" && std::getline(std::cin, query)) {
        if (query == ".quit")
            exit(0);
        if (query.length() == 0)
            continue;

        while (true) {
            std::cout<<"Please choose search mode (0 - Conjunctive Mode, 1 - Disjunctive Mode): \n";
            std::cin>>mode;
            if (mode == "1" || mode == "0")
                break;
        }

        auto start = std::chrono::steady_clock::now();
        bool queryMode = mode == "0";
        auto a = new Query(query, *urltable, *lexicon, *cache, queryMode);

        if (queryMode)
            a->getConjunctiveResult();
        else
            a->getDisjunctiveResult();

        auto end = std::chrono::steady_clock::now();
        std::cout << "Query process time: " << std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count() << " milliseconds.\n";
        delete a;
    }
}

QueryProcessor::QueryProcessor() {
    time_t start = time(nullptr);
    lexicon = new Lexicon(LEXICON_PATH);
    urltable = new URLTable(URL_TABLE_PATH);
    cache = new LRUCache();
    time_t total = difftime(time(nullptr), start);
    std::cout << "Tables load time: " << total << " seconds.\n";
}

QueryProcessor::~QueryProcessor() {
    delete lexicon;
    delete urltable;
    delete cache;
}

