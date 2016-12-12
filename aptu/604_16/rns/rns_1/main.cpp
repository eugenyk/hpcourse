#include <iostream>
#include <fstream>
#include <algorithm>
#include <functional>
#include <numeric>
#include <string>
#include <tbb/flow_graph.h>
#include <boost/program_options.hpp>

#include "image.h"


namespace lab_flow
{
    class generator
    {
    public:
        generator(size_t max_count, size_t width, size_t height)
            : _count(max_count)
            , _width(width)
            , _height(height)
        {}

        bool operator()(lab_flow::image& output)
        {
            if (_count == 0)
            {
                return false;
            }

            output = make_random_image(_width, _height);
            --_count;
            return true;
        }

    private:
        size_t _count;
        size_t _width;
        size_t _height;
    };


    coords_vec value_positions(const lab_flow::image& image, uint8_t value)
    {
        return image.find(value);
    }

    coords_vec max_positions(const lab_flow::image& image)
    {
        lab_flow::image::const_iterator max = std::max_element(image.begin(), image.end());
        return value_positions(image, *max);
    }

    coords_vec min_positions(const lab_flow::image& image)
    {
        lab_flow::image::const_iterator min = std::min_element(image.begin(), image.end());
        //std::cout << "min " << *min << std::endl;
        return value_positions(image, *min);
    }

    void highlight_positions(lab_flow::image& image, const coords_vec& cx)
    {
        for (coords c: cx)
        {
            image[c] = std::numeric_limits<uint8_t>::max();
        }
    }

    image highlight_all(std::tuple<coords_vec, coords_vec,
                                   coords_vec, lab_flow::image> args)
    {
        image img = std::get<3>(args);
        highlight_positions(img, std::get<0>(args));
        highlight_positions(img, std::get<1>(args));
        highlight_positions(img, std::get<2>(args));
        return img;
    }

    lab_flow::image inverse(const lab_flow::image& image)
    {
        lab_flow::image copy = image;
        for (auto& cij : copy)
        {
            cij = std::numeric_limits<uint8_t>::max() - cij;
        }
        return copy;
    }

    double average_brightness(const lab_flow::image& image)
    {
        int total = std::accumulate(image.begin(), image.end(), 0);
        return (double)(total) / (image.get_width() * image.get_height());
    }
}

std::tuple<uint8_t, uint8_t, std::string> get_options(int argc, char** argv)
{
    namespace po = boost::program_options;

    unsigned int value, limit;
    std::string log_name;

    po::options_description desc("TBB Flow graph lab by Nikolay Romashchenko");
    desc.add_options()
        ("help", "Show this message")
        ("brightness,b", po::value<unsigned int>(&value)->default_value(42), "Set target brightness level")
        ("limit,l", po::value<unsigned int>(&limit)->default_value(100), "Set flow size limit")
        ("output,f", po::value<std::string>(&log_name)->default_value("log.txt"), "Log file name")
    ;

    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);    

    if (vm.count("help")) {
        std::cout << desc << "\n";
        exit(1);
    }

    return std::tuple<uint8_t, uint8_t, std::string>((uint8_t)value, (uint8_t)limit, log_name);
}

int main(int argc, char** argv)
{
    using namespace tbb::flow;
    using namespace lab_flow;
    using namespace std::placeholders;

    const size_t num_images = 100;
    const size_t width = 256;
    const size_t height = 256;

    uint8_t value, limit;
    std::string log_name;

    std::tie(value, limit, log_name) = get_options(argc, argv);

    std::cout << "go " << log_name  << " " << int(value) << " " << int(limit) << std::endl;
    std::ofstream fout(log_name);
    graph g;
    generator gen(num_images, width, height);

    source_node<image> source(g, gen, false);
    limiter_node<image> limiter(g, limit, 0);
    broadcast_node<image> broadcast(g);

    make_edge(source, limiter);
    make_edge(limiter, broadcast);

    function_node<image, coords_vec> max_search(g, unlimited, max_positions);
    function_node<image, coords_vec> min_search(g, unlimited, min_positions);
    function_node<image, coords_vec> value_search(g, unlimited, std::bind(value_positions, _1, value));
    make_edge(broadcast, max_search);
    make_edge(broadcast, min_search);
    make_edge(broadcast, value_search);

    join_node<std::tuple<coords_vec, coords_vec, coords_vec, image>, queueing> search_join(g);
    make_edge(max_search, std::get<0>(search_join.input_ports()));
    make_edge(min_search, std::get<1>(search_join.input_ports()));
    make_edge(value_search, std::get<2>(search_join.input_ports()));
    make_edge(broadcast, std::get<3>(search_join.input_ports()));
    
    function_node<std::tuple<coords_vec, coords_vec, coords_vec, image>, image> highlight(g, unlimited, highlight_all);
    broadcast_node<image> broadcast_2(g);
    make_edge(search_join, highlight);
    make_edge(highlight, broadcast_2);

    function_node<image, double> brightness(g, unlimited, average_brightness);
    function_node<image, image> inverse_node(g, unlimited, inverse);

    auto output_func = [&fout](float av_brightness) -> continue_msg {
            fout << "Average brightness: " << av_brightness << std::endl;
            return continue_msg();
    };
    function_node<double, continue_msg> output(g, unlimited, output_func);

    make_edge(broadcast_2, brightness);
    make_edge(broadcast_2, inverse_node);
    make_edge(brightness, output);
    make_edge(output, limiter.decrement);
    
    source.activate();
    g.wait_for_all();
    fout.close();
    
    return 0;
}