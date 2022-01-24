//
// Created by yufan on 10/1/21.
//
#ifndef RINDEXER_CONFIG_H
#define RINDEXER_CONFIG_H


#define LONGEST_WORD_LENGTH 128

#define MAX_TEMP_ENTRIES_PER_FILE 12000000 //30000000 ~500Mb
#define MAX_DOCS_PER_FILE 10000
#define TEMP_FILE_LIMIT 100
#define FILE_MODE_ASCII 1
#define FILE_MODE_BINARY 0
#define INVERTED_LIST_ENTRIES_PER_BLOCK 64
#define MAX_VBYTE_SIZE_UNSIGNED_INT 5

#define MERGE_TEMP_FILES_COMMAND "sort -m -S 1G -k1,1 -k2,2n -t ' ' /home/yufan/CLionProjects/rIndexer/cmake-build-debug/temp/*.txt -o /home/yufan/CLionProjects/rIndexer/cmake-build-debug/merge.txt"
#define MERGED_TEMP_INDEX_PATH "/home/yufan/CLionProjects/rIndexer/cmake-build-debug/merge.txt"
#define DELETE_TEMP_FILES "rm -rf /home/yufan/CLionProjects/rIndexer/cmake-build-debug/temp/*.txt"
#define DELETE_MERGED_POSTINGS "rm -rf /home/yufan/CLionProjects/rIndexer/cmake-build-debug/merge.txt"

#define DEFAULT_PATH "/home/yufan/CLionProjects/rIndexer/cmake-build-debug/fulldocs-new.trec"
#define INVERTED_INDEX_PATH "/home/yufan/CLionProjects/rIndexer/cmake-build-debug/inverted_index.index"
#define LEXICON_TABLE_PATH "/home/yufan/CLionProjects/rIndexer/cmake-build-debug/lexicon.index"
#define URL_TABLE_PATH "/home/yufan/CLionProjects/rIndexer/cmake-build-debug/url_table.index"

#endif
