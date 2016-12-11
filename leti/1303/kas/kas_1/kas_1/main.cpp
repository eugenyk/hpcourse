#include "tbb/flow_graph.h"
//#include <windows.h>
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
        unsigned char* img_map = img->getMap();
        unsigned char max = CHAR_MIN;
        vector<int> arr_max_index;
        int h = img->getHeight();
        int w = img->getWidth();
        for (int i = 0; i < h * w; ++i)
        {
            if (img_map[i] > max)
            {
                max = img_map[i];

                //cleanup arrary of max index
                arr_max_index.clear();
                arr_max_index.push_back(i);
            }
            else if (img_map[i] == max)
            {
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
        unsigned char* img_map = img->getMap();
        unsigned char min = UCHAR_MAX;
        vector<int> arr_min_index;
        int h = img->getHeight();
        int w = img->getWidth();
        for (int i = 0; i < h * w; ++i)
        {
            if (img_map[i] < min)
            {
                min = img_map[i];

                //cleanup arrary of min index
                arr_min_index.clear();
                arr_min_index.push_back(i);
            }
            else if (img_map[i] == min)
            {
                arr_min_index.push_back(i);
            }
        }
        string str = "";
        for (vector<int>::iterator it = arr_min_index.begin(); it != arr_min_index.end(); ++it)
        {
            str += to_string(*it) + " ";
        }
        printf("Min value=%d, Array of index of min value: {%s}\n", min, str.c_str());
        return arr_min_index;
    }
};

class cnt_brightness {
    unsigned char input_brightness;

public:
    cnt_brightness(unsigned char input_brightness) : input_brightness(input_brightness) {}

    vector<int> operator()(Image *img) {
        unsigned char* img_map = img->getMap();
        vector<int> arr_eq_index;
        int h = img->getHeight();
        int w = img->getWidth();
        for (int i = 0; i < h * w; ++i)
        {
            if (img_map[i] == input_brightness)
            {
                arr_eq_index.push_back(i);
            }
        }

        string str = "";
        for (vector<int>::iterator it = arr_eq_index.begin(); it != arr_eq_index.end(); ++it)
        {
            str += to_string(*it) + " ";
        }
        printf("User brightness=%d, Array of vaulue's index equals user brightness: {%s}\n", input_brightness, str.c_str());
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

        //to highlight the adj of max values.
        for (vector<int>::iterator it = arr_max_ind.begin(); it != arr_max_ind.end(); ++it)
        {
            toHighlight(img, *it);
        }

        //to highlight the adj of min values.
        for (vector<int>::iterator it = arr_min_ind.begin(); it != arr_min_ind.end(); ++it)
        {
            toHighlight(img, *it);
        }

        //to highlight the adj of input values.
        for (vector<int>::iterator it = arr_inp_ind.begin(); it != arr_inp_ind.end(); ++it)
        {
            toHighlight(img, *it);
        }
        return img;
    }

    //to highlight the adj pixels.
    void toHighlight(Image* img, int pixelIndex)
    {
        int w = img->getWidth();
        int h = img->getHeight();

        int l = pixelIndex - 1;
        int r = pixelIndex + 1;
        int d = pixelIndex + w;
        int dl = pixelIndex + w - 1;
        int dr = pixelIndex + w + 1;
        int u = pixelIndex - w;
        int ul = pixelIndex - w - 1;
        int ur = pixelIndex - w + 1;

        vector<int> adj_pixs = { u, d };
        //left border.
        if (pixelIndex % w != 0)
        {
            adj_pixs.push_back(ul);
            adj_pixs.push_back(l);
            adj_pixs.push_back(dl);
        }

        //right border.
        if ((pixelIndex + 1) % w != 0)
        {
            adj_pixs.push_back(ur);
            adj_pixs.push_back(r);
            adj_pixs.push_back(dr);
        }

        for (vector<int>::iterator it = adj_pixs.begin(); it != adj_pixs.end(); ++it)
        {
            int cur_pix = *it;
            if (cur_pix >= 0 && cur_pix < w * h)
            {

                img->updPix(cur_pix, 0);
            }
        }
    }
};

struct process_inv {
    Image* operator()(Image *img) {
        Image inv_img = *img;

        unsigned char* img_map = inv_img.getMap();
        int h = inv_img.getHeight();
        int w = inv_img.getWidth();
        for (int i = 0; i < h * w; ++i)
        {
            img_map[i] = UCHAR_MAX - img_map[i];
        }

        printf("Print the inversion image\n");
        inv_img.printMap();

        return &inv_img;
    }
};

class process_avg {
    ofstream &file;

public:
    process_avg(ofstream &file) : file(file) {}

    tuple<int, int> operator()(Image *img) {
        unsigned char* img_map = img->getMap();
        int h = img->getHeight();
        int w = img->getWidth();
        long sum = 0;
        for (int i = 0; i < h * w; ++i)
        {
            sum += img_map[i];
        }
        long avg = sum / (h * w);
        printf("AVG=%d\n", avg);

        //if (file != nullptr)
            //printFile(img->getId(), avg);
        return make_tuple(img->getId(), avg);
    }

    void printFile(int idImage, long avg)
    {
        cout << "*** write to file ***" << endl;
        file << "Id image: #" << idImage << ". Average brightness: " << avg << ".\n";
    }
};

class write_avg {
    ofstream &file;

public:
    write_avg(ofstream &file) : file(file) {}

    int operator()(tuple<int, int> input) {
        int id = get<0>(input);
        int avg = get<1>(input);
        cout << "*** write to file ***" << endl;
        file << "Id image: #" << id << ". Average brightness: " << avg << ".\n";
        return 0;
    }
};



class source_body {
    int n;
    int cur_n;
public:

    source_body(int n) : n(n)
    {
        cur_n = 0;
    }

    bool operator()(Image * &img) {
        if (cur_n < n)
        {
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


int main(int argc, char* argv[]) {
    srand(time(NULL));

    tuple<int, int, int, string> params = Utils::argsProcessing(argc, argv);
    int n = get<0>(params);
    int l = get<1>(params);
    int b = get<2>(params);
    string f = get<3>(params);

    ofstream file;
    if (!f.empty())
    {
        file = ofstream(f, std::ios_base::app);
        file << "---------------------\n";
    }

    graph g;

    source_node<Image*> src_node(g, source_body(n), false);
    limiter_node<Image*> lim_node(g, l);
    function_node<Image*, vector<int>> max_brightness_node(g, unlimited, max_brightness());
    function_node<Image*, vector<int>> min_brightness_node(g, unlimited, min_brightness());
    function_node<Image*, vector<int>> cnt_brightness_node(g, unlimited, cnt_brightness(b));
    join_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, queueing> join(g);
    function_node<tuple<Image*, vector<int>, vector<int>, vector<int>>, Image*> highlight_node(g, unlimited, highlight());
    function_node<Image*, Image*> process_inv_node(g, unlimited, process_inv());
    function_node<Image*, tuple<int, int>> process_avg_node(g, unlimited, process_avg(file));
    function_node<tuple<int, int>, int> write_avg_node(g, serial, write_avg(file));
    join_node<tuple<Image*, int>> eop_join(g);
    function_node<tuple<Image*, int>, continue_msg> eop_node(g, unlimited, eop_body());

    make_edge(src_node, lim_node);
    make_edge(lim_node, max_brightness_node);
    make_edge(lim_node, min_brightness_node);
    make_edge(lim_node, cnt_brightness_node);
    make_edge(lim_node, input_port<0>(join));
    make_edge(max_brightness_node, input_port<1>(join));
    make_edge(min_brightness_node, input_port<2>(join));
    make_edge(cnt_brightness_node, input_port<3>(join));
    make_edge(join, highlight_node);
    make_edge(highlight_node, process_inv_node);
    make_edge(highlight_node, process_avg_node);
    make_edge(process_avg_node, write_avg_node);
    make_edge(process_inv_node, input_port<0>(eop_join));
    make_edge(write_avg_node, input_port<1>(eop_join));
    make_edge(eop_join, eop_node);
    make_edge(eop_node, lim_node.decrement);

    src_node.activate();
    g.wait_for_all();

    if (!f.empty()) file.close();

//    system("pause");
    return 0;
}
