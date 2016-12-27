#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <memory>
#include "tbb/task_group.h"
#include "tbb/flow_graph.h"
#include "pixel.h"
#include "image.h"
#include "program_settings.h"

using JoinNodeType =tbb::flow::tuple<std::shared_ptr<Image>, std::vector<Pixel>, std::vector<Pixel>, std::vector<Pixel>>;

void printHelp()
{
    std::cout << std::endl << "------- Commands ------" << std::endl;
    std::cout << "-n NUMBER - count of images" << std::endl;
    std::cout << "-w NUMBER - width of images" << std::endl;
    std::cout << "-h NUMBER - height of images" << std::endl;
    std::cout << "-l NUMBER - flow limit" << std::endl;
    std::cout << "-b NUMBER - certain brightness" << std::endl;
    std::cout << "-f FILE_NAME - log file" << std::endl;
    std::cout << "-----------------------" << std::endl;
}

void highlight(std::shared_ptr<Image> image, const std::vector<Pixel>& pixels)
{
    size_t row, column;
    for (int i = 0; i < pixels.size(); ++i)
    {
        row = pixels[i].row;
        column = pixels[i].column;
        if (row + 1 < image->getHeight())
        {
            if (column != 0)
            {
                image->setBrightness(row + 1, column - 1, 0);
            }
            image->setBrightness(row + 1, column, 0);
            if (column + 1 < image->getWidth())
            {
                image->setBrightness(row + 1, column + 1, 0);
            }
        }
        if (row != 0)
        {
            if (column != 0)
            {
                image->setBrightness(row - 1, column - 1, 0);
            }
            image->setBrightness(row - 1, column, 0);
            if (column + 1 < image->getWidth())
            {
                image->setBrightness(row - 1, column + 1, 0);
            }
        }
        if (column != 0)
        {
            image->setBrightness(row, column - 1, 0);
        }
        if (column + 1 < image->getWidth())
        {
            image->setBrightness(row, column + 1, 0);
        }
    }
}

int main(int argc, const char** argv)
{
    ProgramSettings settings;
    if (!settings.parse(argc, argv))
    {
        printHelp();
        exit(0);
    }

    tbb::flow::graph graph;

    int image_number = -1;
    tbb::flow::source_node<std::shared_ptr<Image>> generate_image_node(graph, [&image_number, &settings](std::shared_ptr<Image>& image_ptr) mutable {
        Image* image = new Image(settings.image_width, settings.image_height);
        image->generateBrightness();
        image_ptr = std::shared_ptr<Image>(image);
        return ++image_number < settings.image_count;
    });
    tbb::flow::limiter_node<std::shared_ptr<Image>> limit_node(graph, settings.flow_limit);

    tbb::flow::broadcast_node<std::shared_ptr<Image>> broadcast_origin_image_node(graph);

    tbb::flow::function_node<std::shared_ptr<Image>, std::vector<Pixel>> find_min_brightness_node(graph, tbb::flow::unlimited, [](std::shared_ptr<Image> image) {
        std::vector<Pixel> pixels;
        uint8_t min_value = image->getBrightness(0, 0);
        uint8_t curr_value;
        for (size_t row = 0; row < image->getHeight(); ++row)
        {
            for (size_t col = 0; col <  image->getWidth(); ++col)
            {
                curr_value = image->getBrightness(row, col);
                if (curr_value < min_value)
                {
                    min_value = curr_value;
                    pixels.clear();
                    pixels.push_back(Pixel(row, col, min_value));
                }
                else if (curr_value == min_value)
                {
                    pixels.push_back(Pixel(row, col, min_value));
                }
            }
        }
        return pixels;
    });

    tbb::flow::function_node<std::shared_ptr<Image>, std::vector<Pixel>> find_max_brightness_node(graph, tbb::flow::unlimited, [](std::shared_ptr<Image> image) {
        std::vector<Pixel> pixels;
        uint8_t max_value = image->getBrightness(0, 0);
        uint8_t curr_value;
        for (size_t row = 0; row < image->getHeight(); ++row)
        {
            for (size_t col = 0; col <  image->getWidth(); ++col)
            {
                curr_value = image->getBrightness(row, col);
                if (curr_value > max_value)
                {
                    max_value = curr_value;
                    pixels.clear();
                    pixels.push_back(Pixel(row, col, max_value));
                }
                else if (curr_value == max_value)
                {
                    pixels.push_back(Pixel(row, col, max_value));
                }
            }
        }
        return pixels;
    });

    tbb::flow::function_node<std::shared_ptr<Image>, std::vector<Pixel>> find_certain_brightness_node(graph, tbb::flow::unlimited, [&settings](std::shared_ptr<Image> image) {
        std::vector<Pixel> pixels;
        uint8_t brightness = settings.certain_brightness;
        for (size_t row = 0; row < image->getHeight(); ++row)
        {
            for (size_t col = 0; col <  image->getWidth(); ++col)
            {
                if (image->getBrightness(row, col) == brightness)
                {
                    pixels.push_back(Pixel(row, col, brightness));
                }
            }
        }
        return pixels;
    });

    tbb::flow::join_node<JoinNodeType> join_selected_pixels_node(graph);

    tbb::flow::function_node<JoinNodeType, std::shared_ptr<Image>> highlight_node(graph, tbb::flow::unlimited, [](JoinNodeType info) {
        auto matrix = tbb::flow::get<0>(info);
        highlight(matrix, tbb::flow::get<1>(info));
        highlight(matrix, tbb::flow::get<2>(info));
        highlight(matrix, tbb::flow::get<3>(info));
        return matrix;
    });

    tbb::flow::broadcast_node<std::shared_ptr<Image>> broadcast_update_image_node(graph);

    tbb::flow::function_node<std::shared_ptr<Image>, std::shared_ptr<Image>> inverse_image_node(graph, tbb::flow::unlimited, [](std::shared_ptr<Image> image) {
        std::shared_ptr<Image> inverse_image = std::make_shared<Image>(image->getWidth(), image->getHeight());
        for (size_t row = 0; row < image->getHeight(); ++row)
        {
            for (size_t col = 0; col < image->getWidth(); ++col)
            {
                inverse_image->setBrightness(row, col, 255 - image->getBrightness(row, col));
            }
        }
        return inverse_image;
    });

    tbb::flow::function_node<std::shared_ptr<Image>, double> mean_brightness_node(graph, tbb::flow::unlimited, [](std::shared_ptr<Image> image) {
        auto count = image->getHeight() * image->getWidth();
        double mean = 0.0;
        for (size_t row = 0; row < image->getHeight(); ++row)
        {
            for (size_t col = 0; col < image->getWidth(); ++col)
            {
                mean += image->getBrightness(row, col);
            }
        }
        return mean / count;
    });

    tbb::flow::function_node<std::shared_ptr<Image>> image_output_node(graph, tbb::flow::serial, [](std::shared_ptr<Image> image) {
       std::cout << *image << std::endl;
    });

    std::fstream output = std::fstream(settings.log_file, std::fstream::out | std::fstream::trunc);
    tbb::flow::function_node<double> mean_output_node(graph, tbb::flow::serial, [&output](double mean) {
        output << mean << std::endl;
    });

    tbb::flow::function_node<std::shared_ptr<Image>> decrement_node(graph, tbb::flow::serial, [](std::shared_ptr<Image>) {});

    tbb::flow::make_edge(generate_image_node, limit_node);
    tbb::flow::make_edge(limit_node, broadcast_origin_image_node);
    tbb::flow::make_edge(broadcast_origin_image_node, find_min_brightness_node);
    tbb::flow::make_edge(broadcast_origin_image_node, find_max_brightness_node);
    tbb::flow::make_edge(broadcast_origin_image_node, find_certain_brightness_node);
    tbb::flow::make_edge(broadcast_origin_image_node, tbb::flow::input_port<0>(join_selected_pixels_node));
    tbb::flow::make_edge(find_min_brightness_node, tbb::flow::input_port<1>(join_selected_pixels_node));
    tbb::flow::make_edge(find_max_brightness_node, tbb::flow::input_port<2>(join_selected_pixels_node));
    tbb::flow::make_edge(find_certain_brightness_node, tbb::flow::input_port<3>(join_selected_pixels_node));
    tbb::flow::make_edge(join_selected_pixels_node, highlight_node);
    tbb::flow::make_edge(highlight_node, broadcast_update_image_node);
    tbb::flow::make_edge(broadcast_update_image_node, decrement_node);
    tbb::flow::make_edge(decrement_node, limit_node.decrement);
    tbb::flow::make_edge(broadcast_update_image_node, mean_brightness_node);
    tbb::flow::make_edge(broadcast_update_image_node, inverse_image_node);
    tbb::flow::make_edge(inverse_image_node, image_output_node);
    tbb::flow::make_edge(mean_brightness_node, mean_output_node);

    graph.wait_for_all();
    output.close();

    return 0;
}
