cmake_minimum_required(VERSION 3.20)
project(Query)

set(CMAKE_CXX_STANDARD 14)

add_executable(Query main.cpp URLTable.cpp URLTable.h config.h Lexicon.cpp Lexicon.h IndexItem.cpp IndexItem.h Varbyte.cpp Varbyte.h Query.cpp Query.h BM25.cpp BM25.h Snippet.cpp Snippet.h LRUCache.cpp LRUCache.h QueryProcessor.cpp QueryProcessor.h)
