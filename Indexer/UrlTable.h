//
// Created by yufan on 10/13/21.
//

#ifndef RINDEXER_URLTABLE_H
#define RINDEXER_URLTABLE_H

#include <queue>
#include <mutex>
#include "config.h"
#include "unistd.h"

typedef struct pageData {
    unsigned int docId;
    std::string url;
    unsigned int size;
    unsigned long long offset;
} pageData;

void WriteURLs(std::queue<pageData*> *UrlTable, std::mutex *mutex, int mode);


#endif //RINDEXER_URLTABLE_H
