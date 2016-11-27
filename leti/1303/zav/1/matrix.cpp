#include "matrix.h"

Matrix::Matrix(unsigned rows, unsigned cols):m_rows(rows), m_cols(cols), m_values(rows * cols, int(0))
{
}

Matrix::Matrix():m_rows(2), m_cols(2)
{
}

Matrix::Matrix(const Matrix &matrix):m_rows(matrix.getRows()), m_cols(matrix.getCols())
{
    m_values = matrix.data();
}

Matrix &Matrix::operator=(const Matrix &matrix)
{
    m_values = matrix.data();
    m_rows = matrix.getRows();
    m_cols = matrix.getCols();
    return *this;
}

bool Matrix::operator==(const Matrix &matrix) const
{
    if (matrix.getRows() != this->getRows() || matrix.getCols() != this->getCols()) return false;
    return std::equal(matrix.begin(), matrix.end(), this->begin());
}

unsigned Matrix::getCols() const
{
    return m_cols;
}

unsigned Matrix::getRows() const
{
    return m_rows;
}

unsigned Matrix::getSize() const
{
    return m_rows * m_cols;
}

std::vector<unsigned char>::const_iterator Matrix::begin() const
{
    return m_values.cbegin();
}

std::vector<unsigned char>::const_iterator Matrix::end() const
{
    return m_values.cend();
}

void Matrix::setValue(unsigned row, unsigned col, int value)
{
    m_values[row * m_cols + col] = value;
}

std::vector<unsigned char> Matrix::data() const
{
    return m_values;
}

int Matrix::getValue(unsigned row, unsigned col) const
{
    return m_values[row *m_cols + col];
}

std::ostream& operator<<(std::ostream& os, const Matrix& matrix)
{
    for (unsigned i(0); i < matrix.getRows(); ++i) {
        for (unsigned j(0); j < matrix.getCols(); ++j) {
            os << matrix.getValue(i, j) << " ";
        }
        os << "\n";
    }
    return os;
}

std::ostream& operator<<(std::ostream& os, const Index& index)
{
    os << "[" << index.m_row << "," << index.m_col << "]" << std::endl;
    return os;
}

bool Index::operator==(const Index &index) const
{
    return (this->m_row == index.m_row) && (this->m_col == index.m_col);
}
