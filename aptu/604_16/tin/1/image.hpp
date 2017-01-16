#pragma once
#include <cstddef>
#include <vector>
#include <iostream>

using std::size_t;
using std::vector;

class Image {
    size_t rows_;
    size_t cols_;


private:
    int id_;
    vector<vector<int>> data_;
    static constexpr int max_brightness_ = 255;

public:
    Image();
    //Generate random image
    Image(size_t rows, size_t cols, int id);
    Image(int id, const vector<vector<int>>& data);

    void Print() const;
    int GetId() const;
    size_t GetCols() const;
    size_t GetRows() const;

    vector<vector<int>> GetData() const;
    double GetAverage() const;
    void HighlightPixel(size_t rowidx, size_t colidx, size_t square_side);
    Image GetInverted () const;
};
