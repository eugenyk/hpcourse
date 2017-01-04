#include "test.h"

ostream& TestOutput::m_stream = cout;

bool testCompareMatrix(const Matrix &matrixFirst, const Matrix &matrixSecond, bool testResult)
{
    return testResult == (matrixFirst == matrixSecond);
}

bool testCalculateAverageBrightness(Matrix *matrix, double testValue)
{
    return testValue == cast_to<double>(CalcBrightnessNode().operator ()(tagged_msg<size_t, Matrix*>(0, matrix)));
}

bool testCalculateInversionMatrix(Matrix* matrix, Matrix* testInversionMatrix, bool testResult)
{
    Matrix* inversionMatrix = cast_to<Matrix*>(CalcInvMatrixNode().operator ()(tagged_msg<size_t, Matrix*>(0, matrix)));
    return testResult == (*inversionMatrix == *testInversionMatrix);
}

bool testHighlightFoundElementMatrix(Matrix* matrix, Matrix* testMatrix,
                                     vector<Index> firstSetIndex, vector<Index> secondSetIndex, vector<Index> thidSetIndex)
{
    Matrix* newMatrix = cast_to<Matrix*>(HighLightFoundElementMatrix()
                                         .operator()(
                                             make_tuple(
                                                 tagged_msg<size_t, Matrix*>(0, matrix),
                                                 tagged_msg<size_t, vector<Index>>(0, firstSetIndex),
                                                 tagged_msg<size_t, vector<Index>>(0, secondSetIndex),
                                                 tagged_msg<size_t, vector<Index>>(0, thidSetIndex))));
    return *newMatrix == *testMatrix;
}

bool testFindElement(Matrix* matrix, FindElements::TypeSearch typeSearch, int searchElement, std::vector<Index> testIndex)
{
    std::vector<Index> result = cast_to<vector<Index>>(FindElements(typeSearch, searchElement).operator ()(tagged_msg<size_t, Matrix*>(0, matrix)));
    if (result.size() != testIndex.size()) return false;
    return std::equal(result.begin(), result.end(), testIndex.begin());
}

void TestOutput::operator()(tagged_msg<size_t, Matrix*> dataMatrix)
{
    m_stream << "tag = " << dataMatrix.tag() << endl;
    m_stream << "matrix:" << endl;
    Matrix* matrix = cast_to<Matrix*>(dataMatrix);
    m_stream << *matrix;
}

void TestOutput::operator()(tagged_msg<size_t, vector<Index>> dataIndexes)
{
    m_stream << "tag = " << dataIndexes.tag() << endl;
    m_stream <<  "indexes:" << endl;
    std::vector<Index> indexes = cast_to<std::vector<Index>>(dataIndexes);
    for (unsigned i(0); i < indexes.size(); ++i) {
        m_stream << indexes.at(i);
    }
}

tuple<tagged_msg<size_t, Matrix*>,
tagged_msg<size_t, Matrix*>,
tagged_msg<size_t, Matrix*>> TestOutput::operator()(tuple<
                                                    tagged_msg<size_t, Matrix*>,
                                                    tagged_msg<size_t, Matrix*>,
                                                    tagged_msg<size_t, Matrix*>,
                                                    tagged_msg<size_t, vector<Index> >,
                                                    tagged_msg<size_t, vector<Index> >,
                                                    tagged_msg<size_t, vector<Index> >,
                                                    tagged_msg<size_t, double> > data)
{
    m_stream << "start:-----------" << endl;
    this->operator ()(std::get<0>(data));
    m_stream << endl;

    Matrix* matrix = cast_to<Matrix*>(std::get<0>(data));
    vector<Index> maxElements = cast_to<vector<Index>>(std::get<3>(data));
    vector<Index> minElements = cast_to<vector<Index>>(std::get<4>(data));
    vector<Index> customElements = cast_to<vector<Index>>(std::get<5>(data));

    if (!maxElements.empty()) {
        Index firstindex = maxElements.at(0);
        m_stream << "max element = " << matrix->getValue(firstindex.m_row, firstindex.m_col) << endl;
        this->operator ()(std::get<3>(data));
        m_stream << endl;
    }

    if (!minElements.empty()) {
        Index firstindex = minElements.at(0);
        m_stream << "min element = " << matrix->getValue(firstindex.m_row, firstindex.m_col) << endl;
        this->operator ()(std::get<4>(data));
        m_stream << endl;
    }

    if (!customElements.empty()) {
        Index firstindex = customElements.at(0);
        m_stream << "custom element = " << matrix->getValue(firstindex.m_row, firstindex.m_col) << endl;
        this->operator ()(std::get<5>(data));
        m_stream << endl;
    }

    m_stream << "highlight" << endl;
    this->operator ()(std::get<2>(data));
    m_stream << endl;

    m_stream << "tag = " << std::get<1>(data).tag() << endl;
    m_stream << "brightness:" << cast_to<double>(std::get<6>(data)) << endl << endl;

    m_stream << "inversion ";
    this->operator ()(std::get<1>(data));
    m_stream << "end:-----------" << endl << endl;
    return make_tuple(std::get<0>(data), std::get<1>(data), std::get<2>(data));
}
