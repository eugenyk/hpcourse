#ifndef _IMAGE_H_
#define _IMAGE_H_

#include <vector>
#include <fstream>

class Image
{
public:
    Image(size_t w = 0, size_t h = 0);
    ~Image();
    void setBrightness(size_t row, size_t col, uint8_t value);
    uint8_t getBrightness(size_t row, size_t col) const;
    size_t getWidth() const;
    size_t getHeight() const;
    void generateBrightness();

    friend std::ostream& operator<<(std::ostream& output, const Image& image);

private:
    std::vector<uint8_t> _matrix;
    size_t _width;
    size_t _height;
};

#endif // _IMAGE_H_
