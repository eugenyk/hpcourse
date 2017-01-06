#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <atomic>

typedef unsigned char uchar;

struct graph_options
{
    int image_h;
    int image_w;
    unsigned char brightness;
    int max_images;
    bool logging;
    std::string filename;
    graph_options()
    {
        image_h = image_w = brightness = max_images = 0;
        logging = false;
        filename = "";
    }
};

struct pixel
{
    uchar value;
    int h_index;
    int w_index;
};

struct image
{
    uchar* data;
    int height;
    int width;

    int id;

    void generate(int w, int h)
    {
        height = h;
        width = w;
        int size = w*h;
        data = new uchar[size];
        //srand(time(0));
        for(int i = 0; i < size; i++)
        {
            data[i] = rand() % 256;
        }
    }

    uchar at(int h_index, int w_index)
    {
        return data[h_index*width + w_index];
    }

    uchar* pix_ptr(int w_index, int h_index)
    {
        if(w_index < 0 || h_index < 0 ||
                w_index >= width || h_index >= height)
            return 0;
        else
            return &data[h_index*width + w_index];
    }
};

#endif // STRUCTS_H
