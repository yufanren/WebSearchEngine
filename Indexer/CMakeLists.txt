cmake_minimum_required(VERSION 3.20)
project(rIndexer)

set(CMAKE_CXX_STANDARD 14)
SET(CMAKE_CXX_FLAGS -pthread)
add_executable(rIndexer main.cpp FileLoader.cpp FileLoader.h IndexGenerator.cpp IndexGenerator.h config.h Lexicon.h Lexicon.cpp UrlTable.cpp UrlTable.h Varbyte.cpp Varbyte.h)
