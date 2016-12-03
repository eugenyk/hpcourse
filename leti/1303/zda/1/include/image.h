#ifndef IMAGE_H
#define IMAGE_H

#include <vector>

class Image
{
    int _width;
    int _height;
    std::vector< std::vector<unsigned char> > _data;

    void fillRandom();
public:
    Image();
    inline unsigned char at(int x, int y) const {
        if(x < _width && y < _height && x >= 0 && y >= 0) {
            return _data[y][x];
        } else {
            return 0;
        }
    }
    inline void set(int x, int y, unsigned char value) {
        if(x < _width && y < _height && x >= 0 && y >= 0) {
            _data[y][x] = value;
        }
    }

    int width() const;
    int height() const;

    void highlightPoints(std::vector< std::pair<int,int> > points);
};

#endif // IMAGE_H
