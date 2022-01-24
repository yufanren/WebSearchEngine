//
// Created by yufan on 10/13/21.
//
#include <iostream>
#include "UrlTable.h"

//write data to url table.
void WriteURLs(std::queue<pageData*> *UrlTable, std::mutex *mutex, int mode) {

    if(mode == FILE_MODE_ASCII) {
        FILE *fp = fopen(URL_TABLE_PATH, "w");
        while (true) {
            if (UrlTable->empty()) usleep(100);
            else {
                pageData *page = UrlTable->front();
                mutex->lock();
                UrlTable->pop();
                mutex->unlock();
                if (page == nullptr) {
                    fclose(fp);
                    return;
                }
                fprintf(fp, "%u %s %u %llu\n", page->docId, page->url.data(), page->size, page->offset);
                delete page;
            }
        }
    }
    else if (mode == FILE_MODE_BINARY) {

        FILE *fp = fopen(URL_TABLE_PATH, "wb");
        while (true) {
            if (UrlTable->empty()) usleep(100);
            else {
                pageData *page = UrlTable->front();
                mutex->lock();
                UrlTable->pop();
                mutex->unlock();
                if (page == nullptr) {
                    fclose(fp);
                    return;
                }
                fwrite(&page->docId, sizeof(unsigned int), 1, fp);
                size_t urlLength = page->url.length();
                fwrite(&urlLength, sizeof(unsigned int), 1, fp);
                fwrite(page->url.data(), sizeof(char), urlLength, fp);
                fwrite(&page->size, sizeof(unsigned int), 1, fp);
                fwrite(&page->offset, sizeof(unsigned long long), 1, fp);
                delete page;
            }
        }
    }
}
