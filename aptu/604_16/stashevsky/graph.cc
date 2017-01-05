#include "graph.h"
#include "utils.h"

#include <tbb/flow_graph.h>

using namespace std;
using namespace tbb;
using namespace tbb::flow;

template <class T>
static void DoIf(bool predicate, T action) {
    if (predicate) {
        action();
    }
}

static void SetBorder(Image& image, Image const& source, Image::pixel value, size_t i, size_t j) {
    if (source.data[i][j] != value) {
        return;
    }

    auto set = [&image, &source, value](size_t i, size_t j) {
        if (source.data[i][j] != value) {
            image.data[i][j] = UINT8_MAX;
        }
    };

    DoIf(i > 0 && j > 0, [&] { set(i - 1, j - 1); });
    DoIf(i > 0, [&] { set(i - 1, j); });
    DoIf(i > 0 && j < image.width - 1, [&] { set(i - 1, j + 1); });

    DoIf(j > 0, [&] { set(i, j - 1); });
    DoIf(j < image.width - 1, [&] { set(i, j + 1); });

    DoIf(i < image.height - 1 && j > 0, [&] { set(i + 1, j - 1); });
    DoIf(i < image.height - 1, [&] { set(i + 1, j); });
    DoIf(i < image.height - 1 && j < image.width - 1, [&] { set(i + 1, j + 1); });
}

concurrent_vector<double>
Measure(size_t height, size_t width, size_t matrix_count, int equals_value, size_t max_images) {
    graph graph_root;
    broadcast_node<Image> source(graph_root);
    concurrent_vector<double> result;

    function_node<Image, pair < Image::pixel, Image>>
    find_max(graph_root, unlimited, [](Image image) {
        Image::pixel maximum = fold(image, 0, [](Image::pixel a, Image::pixel b) { return max(a, b); });
        return make_pair(maximum, image);
    });

    function_node<Image, pair < uint8_t, Image>>
    find_min(graph_root, unlimited, [](Image image) {
        Image::pixel minimum = fold(image, 255, [](Image::pixel a, Image::pixel b) { return min(a, b); });
        return make_pair(minimum, image);
    });

    function_node<pair < uint8_t, Image>, Image > border(graph_root, unlimited, [](pair <uint8_t, Image> stat) {
        Image result = stat.second;
        Image::pixel value = stat.first;
        for (size_t i = 0; i < stat.second.height; ++i) {
            for (size_t j = 0; j < stat.second.width; ++j) {
                SetBorder(result, stat.second, value, i, j);
            }
        }

        return result;
    });

    broadcast_node<Image> highlighted_caster(graph_root);

    function_node<Image> average(graph_root, unlimited, [&result](Image image) {
        double sum = fold(image, 0.0, [](double prev, Image::pixel item) { return prev + item; });
        result.push_back(sum / (image.width * image.height));
    });

    function_node<Image, Image> inverse(graph_root, unlimited, [](Image image) {
        foreach(image, [](Image::pixel& p) {
            p = static_cast<Image::pixel>(0 - p);
        });

        return image;
    });

    limiter_node<Image> limiter_node(graph_root, max_images);

    make_edge(source, limiter_node);

    make_edge(limiter_node, find_max);
    make_edge(limiter_node, find_min);

    make_edge(find_max, border);
    make_edge(find_min, border);
  
    if (equals_value != -1) {
        function_node<Image, pair < uint8_t, Image>> find_selected(graph_root, unlimited, [equals_value](Image image) {
            return make_pair(equals_value, image);
        });
        make_edge(source, find_selected);
        make_edge(find_selected, border);
    }

    make_edge(border, highlighted_caster);
    make_edge(highlighted_caster, average);
    make_edge(highlighted_caster, inverse);

    for (size_t i = 0; i < matrix_count; ++i) {
        source.try_put(GenerateRandomMatrix(height, width));
    }

    graph_root.wait_for_all();
    return result;
}
