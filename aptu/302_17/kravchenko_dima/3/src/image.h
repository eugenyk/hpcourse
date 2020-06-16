#ifndef __IMAGE_H__
#define __IMAGE_H__

#include <utility>

#define IMAGE_MARK_BOUNDARY 2

typedef std::pair<int, int> point_t;

class image {
public:
    image(int id, int height, int width);

    int get_height();
    int get_width();
    int get_id();

    int get_pixel(int row, int col);
    void set_pixel(int row, int col, int value);
private:
    int *buff;
    int id;
    int height;
    int width;
};

#endif // __IMAGE_H__
