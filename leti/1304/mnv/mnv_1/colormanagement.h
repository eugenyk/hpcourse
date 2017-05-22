#ifndef COLORMANAGEMENT_H
#define COLORMANAGEMENT_H
#include "tbb/flow_graph.h"
#include <iostream>
#include <fstream>
#include <chrono>
#include <ctime>
#include "Image.h"
#include "Utils.h"

using namespace tbb::flow;
using namespace std;

struct max_brightness {
    vector<int> operator()(Image *img) {
        Pixel* img_map = img->getPixelMap();
        Pixel max = CHAR_MIN;
        vector<int> arr_max_index;
        int h = img->getHeight();
        int w = img->getWidth();
        for (int i = 0; i < h * w; ++i) {
            if (img_map[i] > max) {
                max = img_map[i];
                arr_max_index.clear();
                arr_max_index.push_back(i);
            } else if (img_map[i] == max) {
                arr_max_index.push_back(i);
            }
        }
        string str = "";
        for (vector<int>::iterator it = arr_max_index.begin(); it != arr_max_index.end(); ++it)
        {
            str += to_string(*it) + " ";
        }
        printf("Max value=%d, Array of index of max value: {%s}\n", max, str.c_str());
        return arr_max_index;
    }
};

struct min_brightness {
    vector<int> operator()(Image *img) {
        Pixel* img_map = img->getPixelMap();
        Pixel min = UCHAR_MAX;
        vector<int> arr_min_index;
        int h = img->getHeight();
        int w = img->getWidth();
        for (int i = 0; i < h * w; ++i) {
            if (img_map[i] < min) {
                min = img_map[i];
                arr_min_index.clear();
                arr_min_index.push_back(i);
            } else if (img_map[i] == min){
                arr_min_index.push_back(i);
            }
        }
        string str = "";
        for (vector<int>::iterator it = arr_min_index.begin(); it != arr_min_index.end(); ++it) {
            str += to_string(*it) + " ";
        }
        printf("Min value=%d, Array of index of min value: {%s}\n", min, str.c_str());
        return arr_min_index;
    }
};

class cnt_brightness {
    Pixel brightPixel;

public:
    cnt_brightness(Pixel brightPixel);

    vector<int> operator()(Image *img) {
        Pixel* img_map = img->getPixelMap();
        vector<int> arr_eq_index;
        int h = img->getHeight();
        int w = img->getWidth();
        for (int i = 0; i < h * w; ++i) {
            if (img_map[i] == brightPixel)
            {
                arr_eq_index.push_back(i);
            }
        }

        string str = "";
        for (vector<int>::iterator it = arr_eq_index.begin(); it != arr_eq_index.end(); ++it)
        {
            str += to_string(*it) + " ";
        }
        printf("User brightness=%d, Array of vaulue's index equals user brightness: {%s}\n", brightPixel, str.c_str());
        return arr_eq_index;
    }
};

class highlight {
public:
    Image* operator()(tuple<Image*, vector<int>, vector<int>, vector<int>> v) {
        Image* img = get<0>(v);
        vector<int> arr_max_ind = get<1>(v);
        vector<int> arr_min_ind = get<2>(v);
        vector<int> arr_inp_ind = get<3>(v);
        for (vector<int>::iterator it = arr_max_ind.begin(); it != arr_max_ind.end(); ++it){
            toHighlight(img, *it);
        }
        for (vector<int>::iterator it = arr_min_ind.begin(); it != arr_min_ind.end(); ++it){
            toHighlight(img, *it);
        }
        for (vector<int>::iterator it = arr_inp_ind.begin(); it != arr_inp_ind.end(); ++it){
            toHighlight(img, *it);
        }
        return img;
    }

    void toHighlight(Image* img, int pixelIndex);
};

struct process_inv {
    Image* operator()(Image *img) {
        Image inv_img = *img;

        Pixel* img_map = inv_img.getPixelMap();
        int h = inv_img.getHeight();
        int w = inv_img.getWidth();
        for (int i = 0; i < h * w; ++i) {
            img_map[i] = UCHAR_MAX - img_map[i];
        }

        printf("Print the inversion image\n");
        inv_img.printPixelMap();

        return &inv_img;
    }
};

class process_avg {
    ofstream &file;

public:
    process_avg(ofstream &file);

    tuple<int, int> operator()(Image *img) {
        Pixel* img_map = img->getPixelMap();
        int h = img->getHeight();
        int w = img->getWidth();
        long sum = 0;
        for (int i = 0; i < h * w; ++i){
            sum += img_map[i];
        }
        long avg = sum / (h * w);
        printf("AVG=%d\n", avg);
        return make_tuple(img->getId(), avg);
    }

    void printFile(int idImage, long avg);
};

class write_avg {
    ofstream &file;

public:
    write_avg(ofstream &file);

    int operator()(tuple<int, int> input) {
        int id = get<0>(input);
        int avg = get<1>(input);
        cout << "## write to file ##" << endl;
        file << "Id image: #" << id << ". Average brightness: " << avg << ".\n";
        return 0;
    }
};

class source_body {
    int n;
    int cur_n;
public:

    source_body(int n);

    bool operator()(Image * &img) {
        if (cur_n < n){
            img = new Image(10, 10);
            cur_n++;
            return true;
        }
        else return false;
    }
};

struct eop_body {
    continue_msg operator()(const tuple<Image*, int> &input) {
        return continue_msg();
    }
};


#endif // COLORMANAGEMENT_H
