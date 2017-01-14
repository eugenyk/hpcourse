/*
 * How to build: g++ lab_01.cpp -std=c++11 -ltbb -o flowgraph
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <ctime>
#include <iostream>
#include <fstream>
#include <tbb/flow_graph.h>

#define IMAGE_SIZE 1000
#define INVALID_IMAGE_ID -1
#define MAXIMUM_BRIGHT 255
#define MINIMUM_BRIGHT 0

int num_images = 5;
int img_number = 0;

class Point
{
public:
    Point(): x(0), y(0) {}
    Point(int _x, int _y): x(_x), y(_y) {}
    Point(const Point& p): x(p.x), y(p.y) {}

    int x;
    int y;
};

class Image
{
public:
    Image(): id(0) {}
    Image(int size, int _id): id(_id)
    {
        for (int i = 0; i < size; i++)
        {
            std::vector<unsigned char> v;
            for (int j = 0; j < size; j++)
            {
                v.push_back(rand() % 256);
            }
            data.push_back(v);
        }
    }
    Image(const Image& image): id(image.id), data(image.data) {}

    std::vector< std::vector<unsigned char> > data;
    int id;
};

typedef std::pair<Image, std::vector<Point> > detection_pair;
typedef std::tuple<detection_pair, detection_pair, detection_pair > detection_tuple;
typedef std::pair<Image, unsigned char> invert_avg_pair;
typedef std::tuple<invert_avg_pair, invert_avg_pair> output_tuple;

// Настройки приложения
unsigned char searched_bright = 0;
unsigned char image_processing_limit = 0;
std::string log_file_name = "";

void print_help()
{
    fprintf(stderr, "Usage: flowgraph -b searched_bright -l "
        "image_processing_limit [-f log_file_name -n number_of_images]\n");
}

// Обработка входных параметров
bool process_input_args(int argc, char *argv[])
{
    int opt;
    bool b_flag_exist = false;
    bool l_flag_exist = false;
    while ((opt = getopt(argc, argv, "b:l:f:n:")) != -1)
    {
        switch (opt)
        {
        case 'b':
            searched_bright = atoi(optarg);
            b_flag_exist = true;
            break;
        case 'l':
            image_processing_limit = atoi(optarg);
            l_flag_exist = true;
            break;
        case 'f':
            log_file_name = std::string(optarg);
            break;
        case 'n':
            num_images = atoi(optarg);
            break;
        default:
            print_help();
            exit(EXIT_FAILURE);
        }
    }
    return (b_flag_exist && l_flag_exist);
}

Image get_next_image()
{
    if (img_number < num_images)
    {
        img_number++;
        return Image(IMAGE_SIZE, img_number);
    }
    else
    {
        return Image(0, INVALID_IMAGE_ID);
    }
}

std::vector<Point> searchForPixels(Image& image, unsigned char pixel)
{
    std::vector<Point> result;
    for (int i = 0; i < image.data.size(); i++)
    {
        for (int j = 0; j < image.data[i].size(); j++)
        {
            if (image.data[i][j] == pixel)
            {
                result.push_back(Point(i, j));
            }
        }
    }
    return result;
}

int main(int argc, char *argv[])
{
    srand(time(NULL));

    if (!process_input_args(argc, argv))
    {
        print_help();
        exit(EXIT_FAILURE);
    }

    tbb::flow::graph g;

    // Генератор изображений
    tbb::flow::source_node< Image > image_generator(g,
        [](Image &next_image) -> bool
        {
            next_image = get_next_image();
            return (next_image.id != INVALID_IMAGE_ID);
        }
    );

    // Узел, ограничивающий выдачу изображений генератором
    tbb::flow::limiter_node< Image > limiter(g, image_processing_limit);

    // Узел поиска пикселей с максимальной яркостью
    tbb::flow::function_node< Image, detection_pair > maximum_detector(
        g,
        tbb::flow::unlimited,
        [](Image input_image) -> detection_pair
        {
            std::vector<Point> result = searchForPixels(input_image, MAXIMUM_BRIGHT);
            return std::make_pair(input_image, result);
        }
    );

    // Узел поиска пикселей с минимальной яркостью
    tbb::flow::function_node< Image, detection_pair > minimum_detector(
        g,
        tbb::flow::unlimited,
        [](Image input_image) -> detection_pair
        {
            std::vector<Point> result = searchForPixels(input_image, MINIMUM_BRIGHT);
            return std::make_pair(input_image, result);
        }
    );

    // Узел поиска пикселей с яркостью, заданной пользователем
    tbb::flow::function_node< Image, detection_pair > user_detector(
        g,
        tbb::flow::unlimited,
        [](Image input_image) -> detection_pair
        {
            std::vector<Point> result = searchForPixels(input_image, searched_bright);
            return std::make_pair(input_image, result);
        }
    );

    // Узел объединения результатов поиска
    tbb::flow::join_node< detection_tuple, tbb::flow::tag_matching > detectors_join(
        g,
        [](const detection_pair &p) -> int { return (int)p.first.id; },
        [](const detection_pair &p) -> int { return (int)p.first.id; },
        [](const detection_pair &p) -> int { return (int)p.first.id; }
    );

    // Узел обрисовки найденных пикселей рамкой с максимальной яркостью
    tbb::flow::function_node< detection_tuple, Image > draw_borders_node(
        g,
        tbb::flow::unlimited,
        [](const detection_tuple& t) -> Image
        {
            const detection_pair& max_detect = std::get<0>(t);
            const detection_pair& min_detect = std::get<1>(t);
            const detection_pair& usr_detect = std::get<2>(t);
            Image img = max_detect.first;

            std::vector<Point> allPoints (max_detect.second);
            allPoints.insert(allPoints.end(), min_detect.second.begin(), min_detect.second.end());
            allPoints.insert(allPoints.end(), usr_detect.second.begin(), usr_detect.second.end());

            for (int i = 0; i < allPoints.size(); i++)
            {
                const Point& p = allPoints[i];
                // Левый верхний угол
                if (p.x > 0 && p.y > 0) img.data[p.x - 1][p.y - 1] = MAXIMUM_BRIGHT;
                // Пиксел сверху
                if (p.y > 0) img.data[p.x][p.y - 1] = MAXIMUM_BRIGHT;
                // Правый верхний угол
                if (p.x < img.data.size() - 1 && p.y > 0) img.data[p.x + 1][p.y - 1] = MAXIMUM_BRIGHT;
                // Пиксел слева
                if (p.x > 0) img.data[p.x - 1][p.y] = MAXIMUM_BRIGHT;
                // Пиксел справа
                if (p.x < img.data.size() - 1) img.data[p.x + 1][p.y] = MAXIMUM_BRIGHT;
                // Левый нижний угол
                if (p.x > 0 && p.y < img.data[p.x - 1].size() - 1) img.data[p.x - 1][p.y + 1] = MAXIMUM_BRIGHT;
                // Пиксел снизу
                if (p.y < img.data[p.x].size() - 1) img.data[p.x][p.y + 1] = MAXIMUM_BRIGHT;
                // Правый нижний угол
                if (p.x < img.data.size() - 1 && p.y < img.data[p.x + 1].size() - 1) img.data[p.x + 1][p.y + 1] = MAXIMUM_BRIGHT;
            }

            return img;
        }
    );

    // Узел, инвертирующий изображение
    tbb::flow::function_node< Image, invert_avg_pair > invert_node(
        g,
        tbb::flow::unlimited,
        [](Image input_image) -> invert_avg_pair
        {
            for (int i = 0; i < input_image.data.size(); i++)
            {
                for (int j = 0; j < input_image.data[i].size(); j++)
                {
                    input_image.data[i][j] = MAXIMUM_BRIGHT - input_image.data[i][j];
                }
            }
            return std::make_pair(input_image, 0);
        }
    );

    // Узел поиска среднего значения яркости
    tbb::flow::function_node< Image, invert_avg_pair > average_node(
        g,
        tbb::flow::unlimited,
        [](Image input_image) -> invert_avg_pair
        {
            unsigned char result = 0;
            size_t sum = 0;
            size_t divider = IMAGE_SIZE * IMAGE_SIZE;
            for (int i = 0; i < input_image.data.size(); i++)
            {
                for (int j = 0; j < input_image.data[i].size(); j++)
                {
                    sum += (size_t)input_image.data[i][j];
                }
            }
            result = sum / divider;
            return std::make_pair(input_image, result);
        }
    );

    // Узел объединения результатов инверсии изображения и поиска среднего значения яркости.
    tbb::flow::join_node< output_tuple, tbb::flow::tag_matching > invert_average_join(
        g,
        [](const invert_avg_pair &p) -> int { return (int)p.first.id; },
        [](const invert_avg_pair &p) -> int { return (int)p.first.id; }
    );

    // Узел вывода результата.
    tbb::flow::function_node< output_tuple, tbb::flow::continue_msg > output_node(
        g,
        tbb::flow::unlimited,
        [](const output_tuple& t) -> tbb::flow::continue_msg
        {
            invert_avg_pair p = std::get<1>(t);
            unsigned char avg_pixel = p.second;
            if (log_file_name.size() > 0)
            {
                std::ofstream ofs (log_file_name.c_str(), std::ofstream::out);
                ofs << std::to_string(avg_pixel);
                ofs.close();
            }
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
