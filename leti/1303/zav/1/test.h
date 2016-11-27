#ifndef TEST_H
#define TEST_H

#include <string>
#include <ostream>
#include <iostream>
#include <istream>
#include <sstream>

#include <matrix.h>
#include <matrix_algorithm.h>

bool testCalculateAverageBrightness(Matrix *matrix, double testValue);
bool testCalculateInversionMatrix(Matrix *matrix, Matrix *testInversionMatrix, bool testResult);
bool testHighlightFoundElementMatrix(const Matrix& matrix, const Matrix& testMatrix,
                                     vector<Index> firstSetIndex, vector<Index> secondSetIndex,
                                     vector<Index> thidSetIndex);
bool testFindElement(const Matrix& matrix,
                     FindElements::TypeSearch typeSearch,
                     int searchElement, std::vector<Index> testIndex);

struct TestOutput {
    void operator() (tagged_msg<size_t, Matrix*> dataMatrix);
    void operator() (tagged_msg<size_t, vector<Index>> dataIndexes);
    tuple< tagged_msg<size_t, Matrix*>,tagged_msg<size_t, Matrix*>, tagged_msg<size_t, Matrix*>> operator() (tuple<
                            tagged_msg<size_t, Matrix*>,
                            tagged_msg<size_t, Matrix*>,
                            tagged_msg<size_t, Matrix*>,
                            tagged_msg<size_t, vector<Index>>,
                            tagged_msg<size_t, vector<Index>>,
                            tagged_msg<size_t, vector<Index>>,
                            tagged_msg<size_t, double>> data);

    static std::ostream& m_stream;
};

#endif // TEST_H
