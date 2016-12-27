/*
 * How to build: g++ tbb_lab_01.cpp -std=c++11 -ltbb -o flowgraph
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <tbb/flow_graph.h>

const size_t IMAGE_WIDTH = 100;
const size_t IMAGE_HEIGHT = 100;

int num_images = 5;
int img_number = 0;

// Точка
class Point
{
public:
    Point()
    {
        x = y = 0;
    }
    Point(size_t X, size_t Y)
    {
        x = X;
        y = Y;
    }
    size_t x;
    size_t y;
};

// Изображение
class Image
{
public:
    Image()
    {
        width = height = 0;
        img = nullptr;
    }
    Image(size_t w, size_t h)
    {
        width = w;
        height = h;
        img = new unsigned char* [width];
        for (size_t i = 0; i < width; i++)
        {
            img[i] = new unsigned char [height];
            for (size_t j = 0; j < height; j++)
            {
                img[i][j] = rand() % 256;
            }
        }
    }
    Image(const Image& image)
    {
        if (image.img != nullptr)
        {
            img = new unsigned char* [image.width];
            for (size_t i = 0; i < image.width; i++)
            {
                img[i] = new unsigned char [image.height];
                for (size_t j = 0; j < image.height; j++)
                {
                    img[i][j] = image.img[i][j];
                }
            }
        }
    }
    ~Image()
    {
        if (img != nullptr)
        {
            for (size_t i = 0; i < width; i++)
            {
                delete [] img[i];
            }
            delete img;
            img = nullptr;
        }
    }

    unsigned char** img;
    size_t width;
    size_t height;
};

typedef std::pair< Image*, std::vector<Point> > detection_pair;
typedef std::tuple< detection_pair, detection_pair, detection_pair > detection_tuple;
typedef std::tuple< unsigned char, unsigned char > output_tuple;

// Настройки приложения
unsigned char user_brightness = 0;
unsigned char image_processing_limit = 0;
std::string log_file_name = "";

void print_help()
{
    fprintf(stderr, "Usage: flowgraph -b user_brightness -l "
        "image_processing_limit [-f log_file_name]\n");
}

// Обработка входных параметров
bool process_input_args(int argc, char *argv[])
{
    int opt;
    bool b_flag_exist = false;
    bool l_flag_exist = false;
    while ((opt = getopt(argc, argv, "b:l:f:")) != -1)
    {
        switch (opt)
        {
        case 'b':
            user_brightness = atoi(optarg);
            b_flag_exist = true;
            break;
        case 'l':
            image_processing_limit = atoi(optarg);
            l_flag_exist = true;
            break;
        case 'f':
            log_file_name = std::string(optarg);
            break;
        default:
            print_help();
            exit(EXIT_FAILURE);
        }
    }
    return (b_flag_exist && l_flag_exist);
}

Image *get_next_image()
{
    if (img_number < num_images)
    {
        img_number++;
        return new Image(IMAGE_WIDTH, IMAGE_HEIGHT);
    }
    else
    {
        return nullptr;
    }
}

int main(int argc, char *argv[])
{
    if (!process_input_args(argc, argv))
    {
        print_help();
        exit(EXIT_FAILURE);
    }

    tbb::flow::graph g;

    // Source node
    tbb::flow::source_node< Image* > image_generator(g,
        [](Image* &next_image) -> bool
        {
            next_image = get_next_image();
            fprintf(stdout, "=========================================\n");
            if (next_image != nullptr)
            {
                fprintf(stdout, "image_generator: next_image = %p\n", next_image);
                return true;
            }
            else
            {
                fprintf(stdout, "image_generator: next_image = %p, return false\n", next_image);
                return false;
            }
        }
    );

    // Limiter node
    tbb::flow::limiter_node< Image* > limiter(g, image_processing_limit);

    // Maximum detector
    tbb::flow::function_node< Image*, detection_pair > maximum_detector(
        g,
        tbb::flow::unlimited,
        [](Image* input_image) -> detection_pair
        {
            std::vector<Point> result;

            fprintf(stdout, "maximum_detector: input_image = %p\n", input_image);

            return std::make_pair(input_image, result);
        }
    );

    // Minimum detector
    tbb::flow::function_node< Image*, detection_pair > minimum_detector(
        g,
        tbb::flow::unlimited,
        [](Image* input_image) -> detection_pair
        {
            std::vector<Point> result;

            fprintf(stdout, "minimum_detector: input_image = %p\n", input_image);

            return std::make_pair(input_image, result);
        }
    );

    // User detector
    tbb::flow::function_node< Image*, detection_pair > user_detector(
        g,
        tbb::flow::unlimited,
        [](Image* input_image) -> detection_pair
        {
            std::vector<Point> result;

            fprintf(stdout, "user_detector: input_image = %p\n", input_image);

            return std::make_pair(input_image, result);
        }
    );

    // Detectors join node
    tbb::flow::join_node< detection_tuple, tbb::flow::queueing > detectors_join(g);

    // Draw borders node
    tbb::flow::function_node< detection_tuple, Image* > draw_borders_node(
        g,
        tbb::flow::unlimited,
        [](const detection_tuple& t) -> Image*
        {
            const detection_pair& max_detect = std::get<0>(t);
            const detection_pair& min_detect = std::get<1>(t);
            const detection_pair& usr_detect = std::get<2>(t);
            Image* img = max_detect.first;

            fprintf(stdout, "draw_borders_node: input_image = %p\n", img);

            return img;
        }
    );

    // Invert node
    tbb::flow::function_node< Image*, unsigned char > invert_node(
        g,
        tbb::flow::unlimited,
        [](Image* input_image) -> unsigned char
        {
            fprintf(stdout, "invert_node: input_image = %p\n", input_image);
            return 0;
        }
    );

    // Average node
    tbb::flow::function_node< Image*, unsigned char > average_node(
        g,
        tbb::flow::unlimited,
        [](Image* input_image) -> unsigned char
        {
            unsigned char result = 0;
            fprintf(stdout, "average_node: input_image = %p, average pixel = %u\n",
                input_image, result);
            return result;
        }
    );

    // Invert & average join node
    tbb::flow::join_node< output_tuple, tbb::flow::queueing > invert_average_join(g);

    // Output node
    tbb::flow::function_node< output_tuple, tbb::flow::continue_msg > output_node(
        g,
        tbb::flow::unlimited,
        [](const output_tuple& t) -> tbb::flow::continue_msg
        {
            unsigned char avg_pixel = std::get<1>(t);
            fprintf(stdout, "output_node: average pixel = %u\n", avg_pixel);
            return tbb::flow::continue_msg();
        }
    );

    // Link image generator with limiter node
    tbb::flow::make_edge(image_generator, limiter);
    // Link limiter node with detectors
    tbb::flow::make_edge(limiter, maximum_detector);
    tbb::flow::make_edge(limiter, minimum_detector);
    tbb::flow::make_edge(limiter, user_detector);
    // Link detectors with detectors join node
    tbb::flow::make_edge(maximum_detector, tbb::flow::input_port<0>(detectors_join));
    tbb::flow::make_edge(minimum_detector, tbb::flow::input_port<1>(detectors_join));
    tbb::flow::make_edge(user_detector, tbb::flow::input_port<2>(detectors_join));
    // Link detectors join node with draw borders node
    tbb::flow::make_edge(detectors_join, draw_borders_node);
    // Link draw borders node with invert and average nodes
    tbb::flow::make_edge(draw_borders_node, invert_node);
    tbb::flow::make_edge(draw_borders_node, average_node);
    // Link invert and average nodes with its join node
    tbb::flow::make_edge(invert_node, tbb::flow::input_port<0>(invert_average_join));
    tbb::flow::make_edge(average_node, tbb::flow::input_port<1>(invert_average_join));
    // Link invert & average join node with output node
    tbb::flow::make_edge(invert_average_join, output_node);
    // Link output node with limiter decrement port
    tbb::flow::make_edge(output_node, limiter.decrement);

    g.wait_for_all();

    exit(EXIT_SUCCESS);
}
