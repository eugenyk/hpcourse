#include "image.h"
#include "pixel.h"
#include "arguments.h"
#include <tbb/tbb.h>
#include <tbb/flow_graph.h>
#include <tbb/concurrent_vector.h>
#include "graph_types.h"


template<class T>
T findMinValue(image<T> img)
{
    T* buffer = new T[img.height];
    tbb::parallel_for(0, img.height, 1, [buffer, img](size_t i) {
        T* arr = img.data + i*img.width;
        T min = arr[0];
        for(int j = 1; j < img.width; j++)
            if(min > arr[j])
                min = arr[j];
        buffer[i] = min;
    });
    T min = buffer[0];
    for(int i = 1; i < img.height; i++)
        if(min > buffer[i])
            min = buffer[i];
    delete[] buffer;
    return min;
}

template<class T>
T findMaxValue(image<T> img)
{
    T* buffer = new T[img.height];
    tbb::parallel_for(0, img.height, 1, [buffer, img](size_t i) {
        T* arr = img.data + i*img.width;
        T max = arr[0];
        for(int j = 1; j < img.width; j++)
            if(max < arr[j])
                max = arr[j];
        buffer[i] = max;
    });
    T max = buffer[0];
    for(int i = 1; i < img.height; i++)
        if(max < buffer[i])
            max = buffer[i];
    delete[] buffer;
    return max;
}

template<class T>
tbb::concurrent_vector<pixel<T> > findValue(image<T> img, T value)
{
    tbb::concurrent_vector<pixel<T> > result;
    tbb::parallel_for(0, img.height, 1, [img,value, &result](size_t i) {
        T* arr = img.data + i*img.width;
        pixel<T> pix;
        pix.value = value;
        pix.hInd = i;
        for(int j = 0; j < img.width; j++)
            if(arr[j] == value) {
                pix.wInd = j;
                result.push_back(pix);
            }
    });
    return result;
}

template<class T>
void markElement(image<T> img, pixel<T> p)
{
    for(int ii = -1; ii <= 1; ii++)
        for(int jj = -1; jj <= 1; jj++) {
            T* pix_ptr = img.at(p.wInd + jj, p.hInd + ii);
            if(pix_ptr != 0)
                pix_ptr[0] = p.value;
        }
}

void logAvgBrightness(std::vector<avgBrightness8u> b, std::string filename)
{
    FILE* fp = fopen(filename.c_str(), "w+");
    for(int i = 0; i < b.size(); i++)
        fprintf(fp, "Avg Brighness in image %d is %lf\n", b[i].img.id, b[i].avg);
    fclose(fp);
}


int main(int argc, char *argv[])
{
    arguments args;
    if(!parse(argc, argv, args))
        return -1;

    tbb::flow::graph g;
    tbb::flow::broadcast_node<image8u> inputNode(g);
    tbb::flow::function_node<image8u, foundValue8u> findMinValNode(g, args.imagesLimit, [](image8u img) {
        foundValue8u res;
        res.img = img;
        res.val = findMinValue(img);
        return res;
    });
    tbb::flow::function_node<foundValue8u, foundPixels8u> findMinPixelsNode(g, args.imagesLimit, [](foundValue8u fv) {
        foundPixels8u res;
        res.img = fv.img;
        res.vals = findValue(res.img, fv.val);
        return res;
    });
    tbb::flow::function_node<foundPixels8u, image8u> markMinPixelsNode(g, args.imagesLimit, [](foundPixels8u img) {
        tbb::parallel_for(0, int(img.vals.size()), 1, [&img](size_t i) {
            markElement(img.img, img.vals[i]);
        });
        return img.img;
    });
    tbb::flow::function_node<image8u, foundValue8u> findMaxValNode(g, args.imagesLimit, [](image8u img) {
        foundValue8u res;
        res.img = img;
        res.val = findMaxValue(img);
        return res;
    });
    tbb::flow::function_node<foundValue8u, foundPixels8u> findMaxPixelsNode(g, args.imagesLimit, [](foundValue8u fv) {
        foundPixels8u res;
        res.img = fv.img;
        res.vals = findValue(res.img, fv.val);
        return res;
    });
    tbb::flow::function_node<foundPixels8u, image8u> markMaxPixelsNode(g, args.imagesLimit, [](foundPixels8u img) {
        tbb::parallel_for(0, int(img.vals.size()), 1, [&img](size_t i) {
            markElement(img.img, img.vals[i]);
        });
        return img.img;
    });
    tbb::flow::function_node<image8u, foundPixels8u> findPixelsNode(g, args.imagesLimit, [args](image8u img) {
        foundPixels8u res;
        res.img = img;
        res.vals = findValue(img, static_cast<unsigned char>(args.spec));
        return res;
    });
    tbb::flow::function_node<foundPixels8u, image8u> markSpecPixelsNode(g, args.imagesLimit, [](foundPixels8u img) {
        tbb::parallel_for(0, int(img.vals.size()), 1, [&img](size_t i) {
            markElement(img.img, img.vals[i]);
        });
        return img.img;
    });
    tbb::flow::join_node<jointype, tbb::flow::tag_matching> joinNode(g,
        [](const image8u& imag)->int{ return imag.id; },
        [](const image8u& imag)->int{ return imag.id; },
        [](const image8u& imag)->int{ return imag.id; }
    );
    tbb::flow::function_node<jointype, image8u> inverseNode(g, args.imagesLimit, [](jointype t) {
        image8u img = std::get<0>(t);
        tbb::parallel_for(0, img.height, 1, [img](size_t i) {
            for(int j = 0; j < img.width; j++)
                img.data[i*img.width+j] = ~img.data[i*img.width+j];
        });
        return img;
    });
    tbb::flow::function_node<image8u, avgBrightness8u> avgBrightnessNode(g, args.imagesLimit, [](image8u img) {
        avgBrightness8u res;
        res.img = img;
        res.avg = 0;
        double* buffer = new double[img.height];
        tbb::parallel_for(0, img.height, 1, [buffer, img](size_t i) {
            buffer[i] = 0;
            for(int j = 0; j < img.width; j++)
                buffer[i] += img.data[i * img.width + j];
            buffer[i] /= img.width;
        });
        for(int i = 0; i < img.height; i++)
            res.avg += buffer[i];
        res.avg /= img.height;
        delete[] buffer;
        return res;
    });
    tbb::flow::buffer_node<avgBrightness8u> avgResultNode(g);

    tbb::flow::make_edge(inputNode, findMinValNode);
    tbb::flow::make_edge(findMinValNode, findMinPixelsNode);
    tbb::flow::make_edge(findMinPixelsNode, markMinPixelsNode);
    tbb::flow::make_edge(inputNode, findMaxValNode);
    tbb::flow::make_edge(findMaxValNode, findMaxPixelsNode);
    tbb::flow::make_edge(findMaxPixelsNode, markMaxPixelsNode);
    tbb::flow::make_edge(inputNode, findPixelsNode);
    tbb::flow::make_edge(findPixelsNode, markSpecPixelsNode);
    tbb::flow::make_edge(markMinPixelsNode, tbb::flow::input_port<0>(joinNode));
    tbb::flow::make_edge(markMaxPixelsNode, tbb::flow::input_port<1>(joinNode));
    tbb::flow::make_edge(markSpecPixelsNode, tbb::flow::input_port<2>(joinNode));
    tbb::flow::make_edge(joinNode, inverseNode);
    tbb::flow::make_edge(inverseNode, avgBrightnessNode);
    tbb::flow::make_edge(avgBrightnessNode, avgResultNode);

    for(int i = 0; i < args.imagesLimit; i++) {
        image8u img;
        img.id = i;
        img.generate(args.width, args.height);
        inputNode.try_put(img);
    }

    g.wait_for_all();

    std::vector<avgBrightness8u> res;
    for(int i = 0; i < args.imagesLimit; i++) {
        avgBrightness8u v;
        avgResultNode.try_get(v);
        res.push_back(v);
    }

    if(args.logging)
        logAvgBrightness(res, args.filename);

    return 0;
}
