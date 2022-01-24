//
// Created by yufan on 11/3/21.
//

#include "BM25.h"

//f: frequency of term in this document
//n: number of docs containing the term
//N: total number of docs
//dl: length of this document
//avgl: average length of documents in this corpus

double getBM25Score(double f, double n, double N, double dl, double avdl) {
    double k = 1.5, b = 0.75;
    double IDF = log((N - n + 0.5) / (n + 0.5) + 1);
    return IDF * (f * (k + 1) / (f + k * (1 - b + b * dl / avdl)));
}