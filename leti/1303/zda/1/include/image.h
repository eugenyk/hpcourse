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
        if(x < _width && y < _height) {
            return _data.at(y).at(x);
        } else {
            return 0;
        }
    }
    int width() const;
    int height() const;
};

#endif // IMAGE_H
