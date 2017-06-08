//
// Created by michael on 21.05.17.
//


#include <cstdlib>
#include <iostream>

#include "Picture.h"

Picture::Picture(size_t _height, size_t _width) : height(_height), width(_width) {
    pic = new u_char *[height];
    for (size_t i = 0; i < height; i++) {
        pic[i] = new u_char[width];
        for (size_t j = 0; j < width; j++) {
            pic[i][j] = 0;
        }
    }
}

void Picture::fillWithRandomValues() {
    for (size_t i = 0; i < height; i++) {
        for (size_t j = 0; j < width; j++) {
            pic[i][j] = (u_char) rand();
        }
    }
}

void Picture::printToStdout() const {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            std::cout << (int) pic[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

Picture::Picture(const Picture &a) : height(a.height), width(a.width){
    copyPic(*this, a);
}

Picture &Picture::operator=(const Picture &pic) {
    if (this == &pic) {
        return *this;
    }
    this->freeMemory();
    this->height = pic.height;
    this->width = pic.width;
    copyPic(*this, pic);
    return *this;
}

Picture::~Picture() {
    freeMemory();
}


void Picture::freeMemory() {
    for (size_t i = 0; i < height; i++) {
        delete pic[i];
    }
    delete pic;
}

void Picture::copyPic(Picture &a, const Picture &b) {
    a.pic = new u_char *[b.height];
    for (size_t i = 0; i < b.height; i++) {
        a.pic[i] = new u_char[b.width];
        for (size_t j = 0; j < b.width; j++) {
            a.pic[i][j] = b.pic[i][j];
        }
    }
}



