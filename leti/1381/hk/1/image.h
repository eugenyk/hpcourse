#ifndef IMAGE_H
#define IMAGE_H

#include <limits>
#include <random>

template <class T>
struct image
{
    T* data;
    int height;
    int width;
    int id;
    void generate(int w, int h) {
        height = h;
        width = w;
        int size = w*h;
        data = new T[size];
        for(int i = 0; i < size; i++)
            data[i] = rand() % (std::numeric_limits<T>::max() + 1);
    }

    T* at(int w_index, int h_index) {
        if(w_index < 0 || h_index < 0 || w_index >= width || h_index >= height)
            return 0;
        else
            return &data[h_index*width + w_index];
    }
};

#endif // IMAGE_H
