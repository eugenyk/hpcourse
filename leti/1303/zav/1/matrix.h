#ifndef MATRIX_H
#define MATRIX_H

#include <iostream>
#include <vector>

struct Index {
    Index(unsigned row, unsigned col) {
        m_row = row;
        m_col = col;
    }
    Index() {
        m_row = 0;
        m_col = 0;
    }

    bool operator==(const Index& index) const;
    friend std::ostream& operator << (std::ostream &, const Index&);

    unsigned m_row;
    unsigned m_col;
};

class Matrix
{
public:
    Matrix(unsigned rows, unsigned cols);
    Matrix();
    Matrix(const Matrix& matrix);
    Matrix &operator=(const Matrix&);
    bool operator==(const Matrix& matrix) const;

    int getValue(unsigned row, unsigned col) const;
    void setValue(unsigned row, unsigned col, int value);
    void setValue(int index, unsigned char value);

    std::vector<unsigned char> data() const;

    unsigned getCols() const;
    unsigned getRows() const;
    unsigned getSize() const;

    std::vector<unsigned char>::const_iterator begin() const;
    std::vector<unsigned char>::const_iterator end() const;


    friend std::ostream& operator << (std::ostream &, const Matrix&);

protected:
    unsigned m_rows;
    unsigned m_cols;
    std::vector<unsigned char> m_values;
};

#endif // MATRIX_H
