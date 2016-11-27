#include "matrix_algorithm.h"

FindElements::FindElements(FindElements::TypeSearch typeSearch, int searchElement)
{
    m_typeSearch = typeSearch;
    m_searchElement = searchElement;
}

tagged_msg<size_t, std::vector<Index>> FindElements::operator() (tagged_msg<size_t, Matrix*> data)
{
    Matrix* matrix = cast_to<Matrix*>(data);
    switch (m_typeSearch) {
    case TypeSearch::MAX:
        m_searchElement = *std::max_element(matrix->begin(), matrix->end());
        break;
    case TypeSearch::MIN:
        m_searchElement = *std::min_element(matrix->begin(), matrix->end());
        break;
    case TypeSearch::NOTHING:
        return tagged_msg<size_t, vector<Index>>(data.tag(), vector<Index>());
    default:
        break;
    }

    std::vector<Index> indexs;
    for (unsigned i(0); i < matrix->getRows(); ++i) {
        for (unsigned j(0); j < matrix->getCols(); ++j) {
            if (matrix->getValue(i, j) == m_searchElement) {
                indexs.push_back(Index(i, j));
            }
        }
    }
    return tagged_msg<size_t, vector<Index>>(data.tag(), indexs);
}

tagged_msg<size_t, Matrix*> HighLightFoundElementMatrix::operator() (tuple<
                                                                     tagged_msg<size_t, Matrix*>,
                                                                     tagged_msg<size_t, vector<Index>>,
                                                                     tagged_msg<size_t, vector<Index>>,
                                                                     tagged_msg<size_t, vector<Index>>
                                                                     > inputData)
{
    tagged_msg<size_t, Matrix*> message = std::get<0>(inputData);

    Matrix* matrix = new Matrix(*cast_to<Matrix*>(std::get<0>(inputData)));
    vector<Index> maxElem = cast_to<std::vector<Index>>(std::get<1>(inputData));
    vector<Index> minElem = cast_to<std::vector<Index>>(std::get<2>(inputData));
    vector<Index> selectElem = cast_to<std::vector<Index>>(std::get<3>(inputData));

    int maxBrightness = 0;
    int minBrightness = 0;
    int selectBrightness = 0;
    Index index;
    if (!maxElem.empty()) {
        index = maxElem.at(0);
        maxBrightness = matrix->getValue(index.m_row, index.m_col);
    }
    if (!minElem.empty()) {
        index = minElem.at(0);
        minBrightness = matrix->getValue(index.m_row, index.m_col);
    }
    if (!selectElem.empty()) {
        index = selectElem.at(0);
        selectBrightness = matrix->getValue(index.m_row, index.m_col);
    }

    if (!maxElem.empty()) this->highlightMatrix(matrix, maxElem, maxBrightness);
    if (!minElem.empty()) this->highlightMatrix(matrix, minElem, minBrightness);
    if (!selectElem.empty()) this->highlightMatrix(matrix, selectElem, selectBrightness);
    return tagged_msg<size_t, Matrix*>(message.tag(), matrix);
}

void HighLightFoundElementMatrix::highlightMatrix(Matrix* matrix, vector<Index> indexes, int bridghtness)
{
    if (indexes.empty()) return;
    for(unsigned i(0); i < indexes.size(); ++i) {
        Index cI = indexes.at(i);
        unsigned startRow = int(cI.m_row - 1) < 0 ? 0 : cI.m_row - 1;
        unsigned endRow = cI.m_row + 2 > matrix->getRows() ? matrix->getRows() : cI.m_row + 2;
        unsigned startCol = int(cI.m_col - 1) < 0 ? 0 : cI.m_col - 1;
        unsigned endCol = cI.m_col + 2 > matrix->getCols() ? matrix->getCols() : cI.m_col + 2;
        for (unsigned row(startRow); row < endRow; ++row) {
            for (unsigned col(startCol); col< endCol; ++col) {
                matrix->setValue(row, col, bridghtness);
            }
        }
    }
}

tagged_msg<size_t, Matrix*> CalcInvMatrixNode::operator() (tagged_msg<size_t, Matrix*> data)
{
    Matrix* matrix = new Matrix(*cast_to<Matrix*>(data));
    for (unsigned i(0); i < matrix->getRows(); ++i) {
        for (unsigned j(0); j < matrix->getCols(); ++j) {
            int currentValue = matrix->getValue(i, j);
            matrix->setValue(i, j, 255 - currentValue);
        }
    }
    return tagged_msg<size_t, Matrix*>(data.tag(), matrix);
}

tagged_msg<size_t, double> CalcBrightnessNode::operator() (tagged_msg<size_t, Matrix*> data)
{
    Matrix* matrix = cast_to<Matrix*>(data);
    unsigned long long summ = 0;
    for (unsigned i(0); i < matrix->getRows(); ++i) {
        for (unsigned j(0); j < matrix->getCols(); ++j) {
            summ += matrix->getValue(i, j);
        }
    }
    return tagged_msg<size_t, double>(data.tag(), (double)summ/matrix->getSize());
}

OutBrightnessNode::OutBrightnessNode(string filename)
{
    m_filename = filename;
}

void OutBrightnessNode::operator() (tagged_msg<size_t, double> data) {
    ofstream fout(m_filename, ios_base::out | ios_base::app);
    double value = cast_to<double>(data);
    if (fout.is_open()) {
        fout << "tag = " << data.tag() << " brightness:" << value << endl;
        fout.close();
    }
}

void generateMatrix(Matrix* matrix, std::uniform_int_distribution<> &dis, std::mt19937 &gen)
{
    for(unsigned i(0); i < matrix->getRows(); ++i) {
        for(unsigned j(0); j < matrix->getCols(); ++j) {
            matrix->setValue(i, j, dis(gen));
        }
    }
}

continue_msg DeleteMatrix::operator()(tuple<tagged_msg<size_t, Matrix *>, tagged_msg<size_t, Matrix *>, tagged_msg<size_t, Matrix *> > dataMatrix)
{
    clearMemory(cast_to<Matrix*>(std::get<0>(dataMatrix)));
    clearMemory(cast_to<Matrix*>(std::get<1>(dataMatrix)));
    clearMemory(cast_to<Matrix*>(std::get<2>(dataMatrix)));
    return continue_msg();
}

bool DeleteMatrix::clearMemory(Matrix *matrix)
{
    if (matrix != nullptr) {
        delete matrix;
        matrix = nullptr;
        return true;
    } else {
        return false;
    }
}
