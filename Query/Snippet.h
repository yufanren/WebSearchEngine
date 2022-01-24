//
// Created by yufan on 11/4/21.
//

#ifndef QUERY_SNIPPET_H
#define QUERY_SNIPPET_H
#include <iostream>
#include <algorithm>
#include "Query.h"


std::string getSnippet(std::string &text, std::vector<invertedIndexItem*> &terms);
std::pair<int, int> getFirstNear(std::string &text, std::vector<int> &firstTermPos, std::string &term);
std::string getFromPair(std::string &text, std::string &original, std::pair<int, int> pair);
int getBlankBackward(std::string &text, int pos, int a);
int getBlankForward(std::string &text, int pos, int a);
std::string getFromSingle(std::string &text, std::string &original, int a);
int findMiddlePosition(std::string &text, std::string substr);
size_t Find(std::string &text, std::string &substr, int start);
size_t rFind(std::string &text, std::string &substr, int start);
#endif //QUERY_SNIPPET_H
