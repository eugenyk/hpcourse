/*
 * How to build: g++ tbb_lab_01.cpp -std=c++11 -ltbb -o flowgraph
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string>
#include <ctime>
#include <mutex>
#include <iostream>
#include <fstream>
#include <sstream>
#include <tbb/flow_graph.h>

std::mutex g_mutex;

void log(const char* format, ...)
{
    std::lock_guard<std::mutex> lock(g_mutex);
    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);
}

size_t IMAGE_WIDTH = 16;
size_t IMAGE_HEIGHT = 16;

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
        img = r_img = g_img = b_img = nullptr;
        id = 0;
    }
    Image(size_t w, size_t h, size_t ID)
    {
        width = w;
        height = h;
        id = ID;
        img = new unsigned char* [width];
        r_img = new unsigned char* [width];
        g_img = new unsigned char* [width];
        b_img = new unsigned char* [width];
        for (size_t i = 0; i < width; i++)
        {
            img[i] = new unsigned char [height];
            r_img[i] = new unsigned char [height];
            g_img[i] = new unsigned char [height];
            b_img[i] = new unsigned char [height];
            for (size_t j = 0; j < height; j++)
            {
                img[i][j] = rand() % 256;
                r_img[i][j] = g_img[i][j] = b_img[i][j] = img[i][j];
            }
        }
    }
    Image(const Image& image)
    {
        width = image.width;
        height = image.height;
        id = image.id;
        if (image.img != nullptr)
        {
            img = new unsigned char* [image.width];
            r_img = new unsigned char* [image.width];
            g_img = new unsigned char* [image.width];
            b_img = new unsigned char* [image.width];
            for (size_t i = 0; i < image.width; i++)
            {
                img[i] = new unsigned char [image.height];
                r_img[i] = new unsigned char [image.height];
                g_img[i] = new unsigned char [image.height];
                b_img[i] = new unsigned char [image.height];
                for (size_t j = 0; j < image.height; j++)
                {
                    img[i][j] = image.img[i][j];
                    r_img[i][j] = image.r_img[i][j];
                    g_img[i][j] = image.g_img[i][j];
                    b_img[i][j] = image.b_img[i][j];
                }
            }
        }
    }
    Image(Image* image)
    {
        if (image != nullptr && image->img != nullptr)
        {
            width = image->width;
            height = image->height;
            id = image->id;
            img = new unsigned char* [image->width];
            r_img = new unsigned char* [image->width];
            g_img = new unsigned char* [image->width];
            b_img = new unsigned char* [image->width];
            for (size_t i = 0; i < image->width; i++)
            {
                img[i] = new unsigned char [image->height];
                r_img[i] = new unsigned char [image->height];
                g_img[i] = new unsigned char [image->height];
                b_img[i] = new unsigned char [image->height];
                for (size_t j = 0; j < image->height; j++)
                {
                    img[i][j] = image->img[i][j];
                    r_img[i][j] = image->r_img[i][j];
                    g_img[i][j] = image->g_img[i][j];
                    b_img[i][j] = image->b_img[i][j];
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
                delete [] r_img[i];
                delete [] g_img[i];
                delete [] b_img[i];
            }
            delete img;
            delete r_img;
            delete g_img;
            delete b_img;
            r_img = g_img = b_img = img = nullptr;
        }
    }

    unsigned char** img;
    unsigned char** r_img;
    unsigned char** g_img;
    unsigned char** b_img;
    size_t width;
    size_t height;
    size_t id;
};

typedef std::pair< Image*, std::vector<Point> > detection_pair;
typedef std::tuple< detection_pair, detection_pair, detection_pair > detection_tuple;
typedef std::tuple< Image*, unsigned char > output_tuple;

// Настройки приложения
unsigned char user_brightness = 0;
unsigned char image_processing_limit = 0;
std::string log_file_name = "";
unsigned char MAX_BRIGHTNESS = 255;
unsigned char MIN_BRIGHTNESS = 0;

void print_help()
{
    log("Usage: flowgraph -b user_brightness -l "
        "image_processing_limit [-f log_file_name -n num_images -s image_size]\n");
}

// Обработка входных параметров
bool process_input_args(int argc, char *argv[])
{
    int opt;
    bool b_flag_exist = false;
    bool l_flag_exist = false;
    while ((opt = getopt(argc, argv, "b:l:f:n:s:")) != -1)
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
        case 'n':
            num_images = atoi(optarg);
            break;
        case 's':
            IMAGE_WIDTH = IMAGE_HEIGHT = atoi(optarg);
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
        Image* img = new Image(IMAGE_WIDTH, IMAGE_HEIGHT, img_number);
        img_number++;
        return img;
    }
    else
    {
        return nullptr;
    }
}

// Поиск пикселей заданной яркости на изображении
std::vector<Point> findPixels(Image* image, unsigned char value)
{
    std::vector<Point> result;
    if (image != nullptr && image->img != nullptr)
    {
        for (size_t i = 0; i < image->width; i++)
        {
            if (image->img[i] != nullptr)
            {
                for (size_t j = 0; j < image->height; j++)
                {
                    if (image->img[i][j] == value)
                    {
                        result.push_back(Point(i, j));
                    }
                }
            }
        }
    }
    return result;
}

// Печать вектора точек
std::string pointsToString(std::vector<Point> v)
{
    std::stringstream ss;
    ss << '[';
    for (size_t i = 0; i < v.size(); i++)
    {
        ss << '(' << v[i].x << ',' << v[i].y << ')';
    }
    ss << ']';
    return ss.str();
}

void saveImageAsBmp(Image* image, const char* filename)
{
    FILE *f;
    int w = image->width;
    int h = image->height;
    unsigned char *img = NULL;
    int filesize = 54 + 3 * w * h;  //w is your image width, h is image height, both int

    img = (unsigned char *)malloc(3 * w * h);
    memset(img, 0, sizeof(img));

    for(int i = 0; i < w; i++)
    {
        for(int j = 0; j < h; j++)
        {
            int x = i;
            int y = (h - 1) - j;
            int r = image->r_img[i][j];
            int g = image->g_img[i][j];
            int b = image->b_img[i][j];

            img[(x+y*w)*3+2] = (unsigned char)(r);
            img[(x+y*w)*3+1] = (unsigned char)(g);
            img[(x+y*w)*3+0] = (unsigned char)(b);
        }
    }

    unsigned char bmpfileheader[14] = {'B','M', 0,0,0,0, 0,0, 0,0, 54,0,0,0};
    unsigned char bmpinfoheader[40] = {40,0,0,0, 0,0,0,0, 0,0,0,0, 1,0, 24,0};
    unsigned char bmppad[3] = {0,0,0};

    bmpfileheader[ 2] = (unsigned char)(filesize    );
    bmpfileheader[ 3] = (unsigned char)(filesize>> 8);
    bmpfileheader[ 4] = (unsigned char)(filesize>>16);
    bmpfileheader[ 5] = (unsigned char)(filesize>>24);

    bmpinfoheader[ 4] = (unsigned char)(       w    );
    bmpinfoheader[ 5] = (unsigned char)(       w>> 8);
    bmpinfoheader[ 6] = (unsigned char)(       w>>16);
    bmpinfoheader[ 7] = (unsigned char)(       w>>24);
    bmpinfoheader[ 8] = (unsigned char)(       h    );
    bmpinfoheader[ 9] = (unsigned char)(       h>> 8);
    bmpinfoheader[10] = (unsigned char)(       h>>16);
    bmpinfoheader[11] = (unsigned char)(       h>>24);

    f = fopen(filename,"wb");
    fwrite(bmpfileheader,1,14,f);
    fwrite(bmpinfoheader,1,40,f);
    for(int i = 0; i < h; i++)
    {
        fwrite(img+(w*(h-i-1)*3),3,w,f);
        fwrite(bmppad,1,(4-(w*3)%4)%4,f);
    }
    fclose(f);
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

    // Source node ============================================================
    tbb::flow::source_node< Image* > image_generator(g,
        [](Image* &next_image) -> bool
        {
            //std::string filename = "img";
            //filename += std::to_string(img_number);
            //filename += ".bmp";
            next_image = get_next_image();
            //log("=========================================\n");
            if (next_image != nullptr)
            {
                //log("image_generator: next_image = %p\n", next_image);

                //saveImageAsBmp(next_image, filename.c_str());
                return true;
            }
            else
            {
                //log("image_generator: next_image = %p, return false\n", next_image);
                return false;
            }
        }
    );

    // Limiter node ===========================================================
    tbb::flow::limiter_node< Image* > limiter(g, image_processing_limit);

    // Maximum detector =======================================================
    tbb::flow::function_node< Image*, detection_pair > maximum_detector(
        g,
        tbb::flow::serial,
        [](Image* input_image) -> detection_pair
        {
            std::vector<Point> result = findPixels(input_image, MAX_BRIGHTNESS);
            std::string pointsStr = pointsToString(result);
            //log("maximum_detector: input_image = %p, points = %s\n", input_image, pointsStr.c_str());
            return std::make_pair(input_image, result);
        }
    );

    // Minimum detector =======================================================
    tbb::flow::function_node< Image*, detection_pair > minimum_detector(
        g,
        tbb::flow::serial,
        [](Image* input_image) -> detection_pair
        {
            std::vector<Point> result = findPixels(input_image, MIN_BRIGHTNESS);
            std::string pointsStr = pointsToString(result);
            //log("minimum_detector: input_image = %p, points = %s\n", input_image, pointsStr.c_str());
            return std::make_pair(input_image, result);
        }
    );

    // User detector ==========================================================
    tbb::flow::function_node< Image*, detection_pair > user_detector(
        g,
        tbb::flow::serial,
        [](Image* input_image) -> detection_pair
        {
            std::vector<Point> result = findPixels(input_image, user_brightness);
            std::string pointsStr = pointsToString(result);
            //log("user_detector: input_image = %p, points = %s\n", input_image, pointsStr.c_str());
            return std::make_pair(input_image, result);
        }
    );

    // Detectors join node ====================================================
    tbb::flow::join_node< detection_tuple, tbb::flow::tag_matching > detectors_join(
        g,
        [](const detection_pair &p) -> size_t { return (size_t)p.first->id; },
        [](const detection_pair &p) -> size_t { return (size_t)p.first->id; },
        [](const detection_pair &p) -> size_t { return (size_t)p.first->id; }
    );

    // Limiter decrement node =================================================
    tbb::flow::function_node< detection_tuple, tbb::flow::continue_msg > decrement_node(
        g,
        tbb::flow::serial,
        [](const detection_tuple& t) -> tbb::flow::continue_msg
        {
            return tbb::flow::continue_msg();
        }
    );

    // Draw borders node ======================================================
    tbb::flow::function_node< detection_tuple, Image* > draw_borders_node(
        g,
        tbb::flow::serial,
        [](const detection_tuple& t) -> Image*
        {
            static int image_counter = 0;
            const detection_pair& max_detect = std::get<0>(t);
            const detection_pair& min_detect = std::get<1>(t);
            const detection_pair& usr_detect = std::get<2>(t);
            Image* img = max_detect.first;
            std::vector<Point> points;
            points.insert(points.end(), max_detect.second.begin(), max_detect.second.end());
            points.insert(points.end(), min_detect.second.begin(), min_detect.second.end());
            points.insert(points.end(), usr_detect.second.begin(), usr_detect.second.end());

            for (size_t i = 0; i < points.size(); i++)
            {
                const Point& p = points[i];
                // Upper-left
                if (p.x > 0 && p.y > 0)
                {
                    img->img[p.x - 1][p.y - 1] = img->r_img[p.x - 1][p.y - 1] = MAX_BRIGHTNESS;
                    img->g_img[p.x - 1][p.y - 1] = img->b_img[p.x - 1][p.y - 1] = MIN_BRIGHTNESS;
                }
                // Upper
                if (p.y > 0)
                {
                    img->img[p.x][p.y - 1] = img->r_img[p.x][p.y - 1] = MAX_BRIGHTNESS;
                    img->g_img[p.x][p.y - 1] = img->b_img[p.x][p.y - 1] = MIN_BRIGHTNESS;
                }
                // Upper-right
                if (p.y > 0 && p.x < (img->width - 1))
                {
                    img->img[p.x + 1][p.y - 1] = img->r_img[p.x + 1][p.y - 1] = MAX_BRIGHTNESS;
                    img->g_img[p.x + 1][p.y - 1] = img->b_img[p.x + 1][p.y - 1] = MIN_BRIGHTNESS;
                }
                // Left
                if (p.x > 0)
                {
                    img->img[p.x - 1][p.y] = img->r_img[p.x - 1][p.y] = MAX_BRIGHTNESS;
                    img->g_img[p.x - 1][p.y] = img->b_img[p.x - 1][p.y] = MIN_BRIGHTNESS;
                }
                // Right
                if (p.x < (img->width - 1))
                {
                    img->img[p.x + 1][p.y] = img->r_img[p.x + 1][p.y] = MAX_BRIGHTNESS;
                    img->g_img[p.x + 1][p.y] = img->b_img[p.x + 1][p.y] = MIN_BRIGHTNESS;
                }
                // Lower-left
                if (p.x > 0 && p.y < (img->height - 1))
                {
                    img->img[p.x - 1][p.y + 1] = img->r_img[p.x - 1][p.y + 1] = MAX_BRIGHTNESS;
                    img->g_img[p.x - 1][p.y + 1] = img->b_img[p.x - 1][p.y + 1] = MIN_BRIGHTNESS;
                }
                // Lower
                if (p.y < (img->height - 1))
                {
                    img->img[p.x][p.y + 1] = img->r_img[p.x][p.y + 1] = MAX_BRIGHTNESS;
                    img->g_img[p.x][p.y + 1] = img->b_img[p.x][p.y + 1] = MIN_BRIGHTNESS;
                }
                // Lower-right
                if (p.x < (img->width - 1) && p.y < (img->height - 1))
                {
                    img->img[p.x + 1][p.y + 1] = img->r_img[p.x + 1][p.y + 1] = MAX_BRIGHTNESS;
                    img->g_img[p.x + 1][p.y + 1] = img->b_img[p.x + 1][p.y + 1] = MIN_BRIGHTNESS;
                }
            }

            //std::string filename = "selected";
            //filename += std::to_string(image_counter);
            //filename += ".bmp";
            //saveImageAsBmp(img, filename.c_str());
            //image_counter++;

            //log("draw_borders_node: input_image = %p\n", img);

            return img;
        }
    );

    // Invert node ============================================================
    tbb::flow::function_node< Image*, Image* > invert_node(
        g,
        tbb::flow::serial,
        [](Image* input_image) -> Image*
        {
            static int image_counter = 0;
            //log("invert_node: input_image = %p\n", input_image);

            Image invert_image (input_image);
            for (size_t i = 0; i < invert_image.width; i++)
            {
                for (size_t j = 0; j < invert_image.height; j++)
                {
                    invert_image.img[i][j] = MAX_BRIGHTNESS - invert_image.img[i][j];
                    invert_image.r_img[i][j] = MAX_BRIGHTNESS - invert_image.r_img[i][j];
                    invert_image.g_img[i][j] = MAX_BRIGHTNESS - invert_image.g_img[i][j];
                    invert_image.b_img[i][j] = MAX_BRIGHTNESS - invert_image.b_img[i][j];
                }
            }

            //std::string filename = "inverted";
            //filename += std::to_string(image_counter);
            //filename += ".bmp";
            //saveImageAsBmp(&invert_image, filename.c_str());
            //image_counter++;

            return input_image;
        }
    );

    // Average node ===========================================================
    tbb::flow::function_node< Image*, unsigned char > average_node(
        g,
        tbb::flow::serial,
        [](Image* input_image) -> unsigned char
        {
            unsigned char result = 0;
            size_t tmp_result = 0;
            size_t size = input_image->width * input_image->height;
            for (size_t i = 0; i < input_image->width; i++)
            {
                for (size_t j = 0; j < input_image->height; j++)
                {
                    tmp_result += input_image->img[i][j];
                }
            }
            result = tmp_result / size;
            //log("average_node: input_image = %p, average pixel = %u\n", input_image, result);
            return result;
        }
    );

    // Invert & average join node =============================================
    tbb::flow::join_node< output_tuple, tbb::flow::queueing > invert_average_join(g);

    // Output node ============================================================
    tbb::flow::function_node< output_tuple, tbb::flow::continue_msg > output_node(
        g,
        tbb::flow::serial,
        [](const output_tuple& t) -> tbb::flow::continue_msg
        {
            Image* image = std::get<0>(t);
            unsigned char avg_pixel = std::get<1>(t);
            //log("output_node: average pixel = %u\n", avg_pixel);

            if (log_file_name.size() > 0)
            {
                //std::ofstream ofs (log_file_name, std::ofstream::out);
                //ofs << "average pixel value is " << std::to_string(avg_pixel) << std::endl;
                //ofs.close();
            }

            delete image;
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
    // Link detectors join node with draw borders node and limiter decrement node
    tbb::flow::make_edge(detectors_join, draw_borders_node);
    tbb::flow::make_edge(detectors_join, decrement_node);
    // Link decrement node with limiter decrement port
    tbb::flow::make_edge(decrement_node, limiter.decrement);
    // Link draw borders node with invert and average nodes
    tbb::flow::make_edge(draw_borders_node, invert_node);
    tbb::flow::make_edge(draw_borders_node, average_node);
    // Link invert and average nodes with its join node
    tbb::flow::make_edge(invert_node, tbb::flow::input_port<0>(invert_average_join));
    tbb::flow::make_edge(average_node, tbb::flow::input_port<1>(invert_average_join));
    // Link invert & average join node with output node
    tbb::flow::make_edge(invert_average_join, output_node);

    g.wait_for_all();

    exit(EXIT_SUCCESS);
}
