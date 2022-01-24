//
// Created by yufan on 11/4/21.
//

#include "Snippet.h"

//Generate snippet using the 3 most scarce terms among all query terms
//First try to find 2 terms near each other and fetch a whole block surrounding
//them . If fails then get there slices if the text individually.
std::string getSnippet(std::string &text, std::vector<invertedIndexItem*> &terms) {

    std::string target = text;
    std::transform(target.begin(), target.end(), target.begin(),
                   [](unsigned char c) {return std::tolower(c);});

    std::string result;
    int num = terms.size();
    std::vector<int> firstTermPos;
    int firstPos = 0, position;
    while ((position = Find(target, terms[0]->term, firstPos)) != std::string::npos) {
        firstTermPos.push_back(position);
        firstPos = position + 1;
    }
    if (firstTermPos.empty())
        return result;
    if (num > 1) {
        auto pair = getFirstNear(target, firstTermPos, terms[1]->term);
        if (pair.second != 0) {
            result.append(getFromPair(target, text, pair));
            if (num > 2 && Find(result, terms[2]->term, 0) == std::string::npos) {
                result.append("...").append(getFromSingle(target, text, findMiddlePosition(target, terms[2]->term)));
            }
        }
        else if (num > 2) {
            result.append(getFromSingle(target, text, findMiddlePosition(target, terms[1]->term))).append("...");
            pair = getFirstNear(text, firstTermPos, terms[2]->term);
            if (pair.second != 0) {
                result.append(getFromPair(target, text, pair));
            }
            else {
                result.append(getFromSingle(target, text, firstTermPos[firstTermPos.size()/2])).append("...");
                result.append(getFromSingle(target, text, findMiddlePosition(target,terms[2]->term)));
            }
        }
        else {
            result.append(getFromSingle(target, text, firstTermPos[firstTermPos.size()/2])).append("...");
            result.append(getFromSingle(target, text, findMiddlePosition(target, terms[1]->term)));
        }
    }
    else if (num == 1){
        result.append(getFromSingle(target, text, firstTermPos[firstTermPos.size()/2]));
    }

    result.append("...");

    return result;
}

//Try to find a search term near another.
std::pair<int, int> getFirstNear(std::string &text, std::vector<int> &firstTermPos, std::string &term) {
    int firstPos = 0, position;
    int i = 0, p = firstTermPos.size();
    while ((position = Find(text, term, firstPos)) != std::string::npos && i < p) {

        if (position - firstTermPos[i] < - 90) {
            firstPos = position + 1;
        }
        else if (position - firstTermPos[i] > 90) {
            i++;
        }
        else {
            return position < firstTermPos[i] ? std::pair<int, int>{position, firstTermPos[i]} : std::pair<int, int>{firstTermPos[i], position};
        }
    }
    return {0, 0};
}

//Get a snippet from a single term hit.
std::string getFromSingle(std::string &text, std::string &original, int a) {
    if (a == std::string::npos)
        return "";
    int posa = text.rfind('\n', a);
    int posb= getBlankBackward(text, a, 7);
    int pos1 = std::max(posa, posb);
    if (a - pos1 > 60)
        pos1 = a - 60;

    posa = text.find('\n', a);
    posb = getBlankForward(text, a, 7);
    int pos2 = std::min(posa, posb);
    if (pos2 - a > 60)
        pos2 = a + 60;

    return original.substr(std::max(pos1, 0), std::min(pos2 - pos1, (int)text.length()-1));
}

//Get a snippet from 2 terms near each other.
std::string getFromPair(std::string &text,  std::string &original, std::pair<int, int> pair) {
    if (pair.second == std::string::npos)
        return "";
    int posa = text.rfind('\n', pair.first);
    int posb= getBlankBackward(text, pair.first, 7);
    int pos1 = std::max(posa, posb);
    if (pair.first - pos1 > 60)
        pos1 = pair.first - 60;

    posa = text.find('\n', pair.second);
    posb = getBlankForward(text, pair.second, 7);
    int pos2 = std::min(posa, posb);
    if (pos2 - pair.second > 60)
        pos2 = pair.second + 60;

    return original.substr(std::max(pos1, 0),std::min(pos2 - pos1, (int)text.length()-1));
}

//Get the nth space char before a position
//for finding a cutoff point for a snippet.
int getBlankBackward(std::string &text, int pos, int a) {
    while (a > 0) {
        pos = text.rfind(' ', pos);
        if (pos == std::string::npos)
            return 0;
        pos--;
        a--;
    }
    return std::max(pos + 1, 0);
}

//Get the nth space char after position
//for finding a cutoff point for a snippet.
int getBlankForward(std::string &text, int pos, int a) {
    int length = text.length() - 1;
    while (a > 0) {
        pos = text.find(' ', pos);
        if (pos == std::string::npos)
            return length;
        pos++;
        a--;
    }
    return std::min(pos - 1, length);
}

//Try to find a search term, starting form the middle of the text.
int findMiddlePosition(std::string &text, std::string substr) {
    int position = text.length() / 2;
    int result = rFind(text, substr, position);
    if (result == std::string::npos)
        result = Find(text, substr, position);
    return result;
}

//A wrapper function for std::find()
//Make sure the term found is not substring of another term.
size_t Find(std::string &text, std::string &substr, int start) {

    int position = start, length = substr.length(), totalLength = text.length();
    while ((position = text.find(substr, position)) != std::string::npos) {
        if ((position == 0 || !std::isalnum(text.at(position - 1))) && (position + length >= totalLength || !std::isalnum(text.at(position+length)))) {
            return position;
        }
        else {
            if (position >= totalLength - 1)
                return std::string::npos;
            else
                position++;
        }
    }
    return std::string::npos;
}

//A wrapper function for std::rfind()
//Make sure the term found is not substring of another term.
size_t rFind(std::string &text, std::string &substr, int start) {

    int position = start, length = substr.length(), totalLength = text.length();
    while ((position = text.rfind(substr, position)) != std::string::npos) {
        if ((position == 0 || !std::isalnum(text.at(position - 1))) && (position + length >= totalLength || !std::isalnum(text.at(position+length)))) {
            return position;
        }
        else {
            if (position <= 0)
                return std::string::npos;
            else
                position--;
        }
    }
    return std::string::npos;
}