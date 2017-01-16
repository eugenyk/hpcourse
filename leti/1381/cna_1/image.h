#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

struct Coord2d {
    std::ptrdiff_t column;
    std::ptrdiff_t row;
};

Coord2d fromDiffType(std::ptrdiff_t diff, std::size_t nColumn, std::size_t nRow);
std::ptrdiff_t toDiffType(const Coord2d &coord, std::size_t nColumn, std::size_t nRow);
bool isValid(const Coord2d &coord, std::size_t nColumn, std::size_t nRow);

template <typename DiffTypeMarkContainer, typename MarkContainer>
void mark(DiffTypeMarkContainer dtmc, MarkContainer &container, const std::size_t columns, const std::size_t rows, typename MarkContainer::value_type mark)
{
    for(typename DiffTypeMarkContainer::const_iterator diffIt = dtmc.begin(); diffIt != dtmc.end(); diffIt++) {
        const Coord2d &coord = fromDiffType(*diffIt, columns, rows);
        for(std::ptrdiff_t diffColumn = -1; diffColumn <= 1; diffColumn++) {
            for(std::ptrdiff_t diffRow = -1; diffRow <= 1; diffRow++) {
                if(diffColumn == 0 && diffRow == 0)
                    continue;
                Coord2d c = coord;
                c.column += diffColumn;
                c.row += diffRow;
                if(isValid(c, columns, rows)) {
                    *(container.begin() + toDiffType(c, columns, rows)) = mark;
                }
            }
        }
    }
}

class Image
{
public:
    typedef std::vector<unsigned char> Container;

    Image(unsigned int width = 0, unsigned int height = 0);

    inline unsigned int width() const { return m_width; }
    inline unsigned int height() const { return m_height; }
    inline unsigned int size() const { return width() * height(); }

    void fill();

    inline Container &pixels() { return m_pixels; }
    inline const Container &pixels() const { return m_pixels; }
    inline const Container &cpixels() const { return m_pixels; }

private:
    Container m_pixels;
    unsigned int m_width, m_height;
};

#endif // IMAGE_H
