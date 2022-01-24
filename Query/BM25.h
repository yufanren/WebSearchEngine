//
// Created by yufan on 11/3/21.
//

#ifndef QUERY_BM25_H
#define QUERY_BM25_H
#include <cmath>

double getBM25Score(double n, double f, double N, double dl, double avdl);

#endif //QUERY_BM25_H
