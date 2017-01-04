#ifndef MATRIX_ALGORITHM_H
#define MATRIX_ALGORITHM_H

#include "matrix.h"
#include "tbb/flow_graph.h"

#include <iostream>
#include <fstream>
#include <tuple>
#include <algorithm>

using namespace std;
using namespace tbb::flow;

void generateMatrix(Matrix* matrix, std::uniform_int_distribution<> &dis, std::mt19937 &gen);

struct FindElements {
    enum class TypeSearch {
        MAX,
        MIN,
        CUSTOM,
        NOTHING
    };

    FindElements(FindElements::TypeSearch typeSearch = FindElements::TypeSearch::MAX, int searchElement = 0);
    tagged_msg<size_t, std::vector<Index>> operator() (tagged_msg<size_t, Matrix*> matrix);

    TypeSearch m_typeSearch;
    int m_searchElement;
};

struct HighLightFoundElementMatrix {
    tagged_msg<size_t, Matrix*> operator() (tuple<
                                           tagged_msg<size_t, Matrix*>,
                                           tagged_msg<size_t, vector<Index>>,
                                           tagged_msg<size_t, vector<Index>>,
                                           tagged_msg<size_t, vector<Index>>
                                           > inputData);
    void highlightMatrix(Matrix* matrix, vector<Index> indexes, int bridghtness);
};

struct CalcInvMatrixNode {
    tagged_msg<size_t, Matrix*> operator() (tagged_msg<size_t, Matrix*> dataMatrix);
};

struct CalcBrightnessNode {
    tagged_msg<size_t, double> operator() (tagged_msg<size_t, Matrix*> dataMatrix) ;
};

struct OutBrightnessNode {
    OutBrightnessNode(std::string filename);
    void operator() (tagged_msg<size_t, double> value);

    std::string m_filename;
};

struct DeleteMatrix {
    continue_msg operator() (tuple<
                     tagged_msg<size_t, Matrix*>,
                     tagged_msg<size_t, Matrix*>,
                     tagged_msg<size_t, Matrix*>
                     > dataMatrix);

    bool clearMemory(Matrix *matrix);
};

#endif // MATRIX_ALGORITHM_H
