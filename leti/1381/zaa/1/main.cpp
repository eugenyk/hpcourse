#include <iostream>
#include <memory>
#include "tbb/flow_graph.h"
#include "image.h"

using namespace tbb::flow;
using Pixel = std::tuple<int, int>;

int image_count = 10;
int limit = 5;
std::string log_file = "log.txt";
int desired_brightness = -1;

bool parseCommandLine(int argc, const char** argv)
{
    for(int i = 1; i < argc; i += 2) {
        if ((strcmp(argv[i], "-c") == 0)) {
            image_count = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-l") == 0)) {
            limit = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-b") == 0)) {
            desired_brightness = std::atoi(argv[i+1]);
        }
        else if ((strcmp(argv[i], "-f") == 0)) {
            log_file = argv[i+1];
        }
        else return false;
    }
    return true;
}

void printUsage()
{
    std::cout << "======== Usage =======\n";
    std::cout << "-c - count of images\n";
    std::cout << "-l - thread limit\n";
    std::cout << "-b - desired brightness\n";
    std::cout << "-f - log file\n";
    std::cout << "====================\n";
}

void highlight(std::shared_ptr<Image> image, const std::vector<Pixel>& pixels)
{
    for (int i = 0; i < pixels.size(); i++) {
        int row = std::get<0>(pixels[i]);
        int col = std::get<1>(pixels[i]);
        if (row + 1 < image->height) {
            if (col != 0)
                image->pixels[(row + 1) * image->width + (col - 1)] = 0;
            image->pixels[(row + 1) * image->width + col] = 0;
            if (col + 1 < image->width)
                image->pixels[(row + 1) * image->width + (col + 1)] = 0;
        }
        if (col != 0)
            image->pixels[row * image->width + (col - 1)] = 0;
        if (col + 1 < image->width)
            image->pixels[row * image->width + (col + 1)] = 0;
        if (row != 0) {
            if (col != 0)
                image->pixels[(row - 1) * image->width + (col - 1)] = 0;
            image->pixels[(row - 1) * image->width + col] = 0;
            if (col + 1 < image->width)
                image->pixels[(row - 1) * image->width + (col + 1)] = 0;
        }
    }
}

int main(int argc, const char** argv)
{
    if (!parseCommandLine(argc, argv)) {
        printUsage();
        exit(0);
    }

    graph g;
    int task_id = 0;
    source_node<std::shared_ptr<Image>> image_generate_node(g, [&task_id](std::shared_ptr<Image>& image) mutable {
        Image* img = new Image(25, 25);
        img->random();
        img->key = task_id;
        image = std::shared_ptr<Image>(img);
        return task_id++ < image_count;
    });


    limiter_node<std::shared_ptr<Image>> limit_node(g, limit);

    broadcast_node<std::shared_ptr<Image>> broadcast_origin_image_node(g);

    using PixelsOf = std::tuple<int, std::vector<Pixel>>;

    function_node<std::shared_ptr<Image>, PixelsOf> find_min_node(g, unlimited, [](std::shared_ptr<Image> image) {
        std::vector<Pixel> pixels;
        Brightness min = image->pixels[0];
        Brightness curr;
        for (int i = 0; i < image->height; i++) {
            for (int j = 0; j <  image->width; j++) {
                curr = image->pixels[i * image->width + j];
                if (curr < min) {
                    min = curr;
                    pixels.clear();
                    pixels.push_back(Pixel(i, j));
                }
                else if (curr == min) {
                    pixels.push_back(Pixel(i, j));
                }
            }
        }
        return PixelsOf(image->key, pixels);
    });

    function_node<std::shared_ptr<Image>, PixelsOf> find_max_node(g, unlimited, [](std::shared_ptr<Image> image) {
        std::vector<Pixel> pixels;
        Brightness max = image->pixels[0];
        Brightness curr;
        for (int i = 0; i < image->height; i++) {
            for (int j = 0; j < image->width; j++) {
                curr = image->pixels[i * image->width + j];
                if (curr > max) {
                    max = curr;
                    pixels.clear();
                    pixels.push_back(Pixel(i, j));
                }
                else if (curr == max) {
                    pixels.push_back(Pixel(i, j));
                }
            }
        }
        return PixelsOf(image->key, pixels);
    });

    function_node<std::shared_ptr<Image>, PixelsOf> find_desired_node(g, unlimited, [](std::shared_ptr<Image> image) {
        std::vector<Pixel> pixels;
        if (desired_brightness >= 0) {
            Brightness b = desired_brightness;
            for (int i = 0; i < image->height; i++) {
                for (int j = 0; j <  image->width; j++) {
                    if (image->pixels[i * image->width + j] == b) {
                        pixels.push_back(Pixel(i, j));
                    }
                }
            }
        }
        return PixelsOf(image->key, pixels);
    });

    using SelectedPixels = tbb::flow::tuple<std::shared_ptr<Image>, PixelsOf, PixelsOf, PixelsOf>;

    join_node<SelectedPixels, tbb::flow::tag_matching> join_selected_pixels_node(g,
        [](std::shared_ptr<Image> image)->int {return image->key;},
        [](const PixelsOf& p)->int {return std::get<0>(p);},
        [](const PixelsOf& p)->int {return std::get<0>(p);},
        [](const PixelsOf& p)->int {return std::get<0>(p);}
        );

    function_node<SelectedPixels, std::shared_ptr<Image>> highlight_node(g, unlimited, [](SelectedPixels pixels) {
        auto matrix = tbb::flow::get<0>(pixels);
        highlight(matrix, std::get<1>(tbb::flow::get<1>(pixels)));
        highlight(matrix, std::get<1>(tbb::flow::get<2>(pixels)));
        highlight(matrix, std::get<1>(tbb::flow::get<3>(pixels)));
        return matrix;
    });

    broadcast_node<std::shared_ptr<Image>> broadcast_update_image_node(g);

    function_node<std::shared_ptr<Image>, std::shared_ptr<Image>> inverse_image_node(g, serial, [](std::shared_ptr<Image> image) {
        std::shared_ptr<Image> inverse_image = std::make_shared<Image>(image->width, image->height);
        for (int i = 0; i < image->height; i++) {
            for (int j = 0; j < image->width; j++) {
                inverse_image->pixels[i * inverse_image->width + j] = 255 - image->pixels[i * image->width + j];
            }
        }
        return inverse_image;
    });

    function_node<std::shared_ptr<Image>, double> mean_node(g, serial, [](std::shared_ptr<Image> image) {
        int count = image->height * image->width;
        double mean = 0.0;
        for (int i = 0; i < image->height; i++) {
            for (int j = 0; j < image->width; j++) {
                mean += image->pixels[i * image->width + j];
            }
        }
        return mean / count;
    });

    function_node<std::shared_ptr<Image>> image_output_node(g, serial, [](std::shared_ptr<Image> image) {
       std::cout << *image << "\n";
    });

    std::fstream out = std::fstream(log_file, std::fstream::out | std::fstream::trunc);
    tbb::flow::function_node<double> mean_output_node(g, serial,
        [&out](double mean) {
            out << mean << "\n";
    });

    function_node<std::shared_ptr<Image>> decrement_node(g, serial, [](std::shared_ptr<Image>) {});

    make_edge(image_generate_node, limit_node);
    make_edge(limit_node, broadcast_origin_image_node);
    make_edge(broadcast_origin_image_node, find_min_node);
    make_edge(broadcast_origin_image_node, find_max_node);
    make_edge(broadcast_origin_image_node, find_desired_node);

    make_edge(broadcast_origin_image_node, input_port<0>(join_selected_pixels_node));
    make_edge(find_min_node, input_port<1>(join_selected_pixels_node));
    make_edge(find_max_node, input_port<2>(join_selected_pixels_node));
    make_edge(find_desired_node, input_port<3>(join_selected_pixels_node));
    make_edge(join_selected_pixels_node, highlight_node);

    make_edge(highlight_node, broadcast_update_image_node);
    make_edge(broadcast_update_image_node, decrement_node);
    make_edge(decrement_node, limit_node.decrement);

    make_edge(broadcast_update_image_node, mean_node);
    make_edge(broadcast_update_image_node, inverse_image_node);

    make_edge(inverse_image_node, image_output_node);
    make_edge(mean_node, mean_output_node);

    g.wait_for_all();

    out.close();

    return 0;
}