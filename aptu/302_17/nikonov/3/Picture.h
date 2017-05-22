//
// Created by michael on 21.05.17.
//

#ifndef HW03_PICTURE_H
#define HW03_PICTURE_H


#include <cstddef>
#include <tuple>

typedef std::tuple<size_t, size_t> Pnt;

class Picture {
public:
    u_char** pic = nullptr;
    size_t height;
    size_t width;

    Picture(size_t _height = 0, size_t _width = 0);
    void fillWithRandomValues();
    void printToStdout() const;
    virtual ~Picture();
    Picture(const Picture& a);
    Picture& operator=(const Picture& pic);

private:
    void freeMemory();
    static void copyPic(Picture& a, const Picture& b);
};




#endif //HW03_PICTURE_H
