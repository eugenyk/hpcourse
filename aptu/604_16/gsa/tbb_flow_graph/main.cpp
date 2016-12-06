#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstddef>
#include <utility>
#include <unistd.h>

#include <tbb/flow_graph.h>

using namespace std;
using namespace tbb::flow;


#define M 256
#define N 256

using image = vector<vector<uint8_t>>;
using position = pair<int, int>;
using positions_vec = vector<position>;

struct options
{
    uint8_t pixel_value = 0;
    size_t n_images = 0;
    string output_file_path;
};


void print_usage(const char* prog_name)
{
    std::cout << "USAGE:" << std::endl;
    std::cout << prog_name << " <options>" << std::endl;
    std::cout << "options: " << std::endl;
    std::cout << "  -b <number> - brightness value for search ([0, 255])" << std::endl;
    std::cout << "  -l <number> - max number of images" << std::endl;
    std::cout << "  -f <path> - output file path" << std::endl;
}

options get_options(int argc, char** argv)
{
    if (argc != 7)
    {
        print_usage(argv[0]);
        exit(0);
    }

    options opt;

    for (int i = 1; i < argc; ++i)
    {
        string param(argv[i]);

        if (param == "-b")
        {
            ++i;
            int temp = atoi(argv[i]);
            if (temp < 0 || temp > 255)
            {
                print_usage(argv[0]);
                exit(0);
            }
            else
            {
                opt.pixel_value = temp;
            }
        }
        else if(param == "-l")
        {
            ++i;
            int temp = atoi(argv[i]);
            if (temp < 0)
            {
                print_usage(argv[0]);
                exit(0);
            }
            else
            {
                opt.n_images = temp;
            }

        }
        else if (param == "-f")
        {
            ++i;
            opt.output_file_path = argv[i];
        }
        else
        {
            print_usage(argv[0]);
            exit(0);
        }
    }

    return opt;
}

int main(int argc, char** argv)
{
    srand(2016);
    const options opt = get_options(argc, argv);

    graph g;

    size_t img_counter = 0;
    auto img_generator_lambda = [&opt, &img_counter] (image& img) {
        img.resize(M);
        for (int i = 0; i < M; ++i)
        {
            img[i].resize(N);
            for (int j = 0; j < N; ++j)
            {
                img[i][j] = rand();
            }
        }
        ++img_counter;
        return img_counter < opt.n_images;
    };

    source_node<image> img_generator(g, img_generator_lambda, false);


    limiter_node<image> limiter(g, opt.n_images);
    make_edge(img_generator, limiter);


    auto max_val_finder_lambda = [](const image& img) {
        positions_vec res;
        int max_val = -1;
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                if (img[i][j] == max_val)
                {
                    res.push_back(position(i, j));
                }
                else if (img[i][j] > max_val)
                {
                    max_val = img[i][j];
                    res.clear();
                    res.push_back(position(i, j));
                }
            }
        }

        return res;
    };

    function_node<image, positions_vec> max_val_finder(g, 1, max_val_finder_lambda);
    make_edge(limiter, max_val_finder);


    auto min_val_finder_lambda = [](const image& img) {
        positions_vec res;
        int min_val = 256;
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                if (img[i][j] == min_val)
                {
                    res.push_back(position(i, j));
                }
                else if (img[i][j] < min_val)
                {
                    min_val = img[i][j];
                    res.clear();
                    res.push_back(position(i, j));
                }
            }
        }

        return res;
    };

    function_node<image, positions_vec> min_val_finder(g, 1, min_val_finder_lambda);
    make_edge(limiter, min_val_finder);


    auto val_finder_lambda = [&opt](const image& img) {
        positions_vec res;
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                if (img[i][j] == opt.pixel_value)
                {
                    res.push_back(position(i, j));
                }
            }
        }

        return res;
    };

    function_node<image, positions_vec> val_finder(g, 1, val_finder_lambda);
    make_edge(limiter, val_finder);


    join_node<std::tuple<image, positions_vec, positions_vec, positions_vec>> features_joiner(g);

    make_edge(limiter, input_port<0>(features_joiner));
    make_edge(max_val_finder, input_port<1>(features_joiner));
    make_edge(min_val_finder, input_port<2>(features_joiner));
    make_edge(val_finder, input_port<3>(features_joiner));


    auto marker_lambda = [](const std::tuple<image, positions_vec, positions_vec, positions_vec>& data) {
        const positions_vec& v1 = std::get<1>(data);
        const positions_vec& v2 = std::get<2>(data);
        const positions_vec& v3 = std::get<3>(data);
        image img = std::get<0>(data);

        positions_vec all_pos(v1.cbegin(), v1.cend());
        all_pos.insert(all_pos.end(), v2.cbegin(), v2.cend());
        all_pos.insert(all_pos.end(), v3.cbegin(), v3.cend());

        for (int i = 0; i < all_pos.size(); ++i)
        {
            int x = all_pos[i].first;
            int y = all_pos[i].second;

            if (x > 1)
                img[x-1][y] = 255;
            if (x < M-1)
                img[x+1][y] = 255;
            if (y > 1)
                img[x][y-1] = 255;
            if(y < N-1)
                img[x][y+1] = 255;
        }

        return img;
    };

    function_node<std::tuple<image, positions_vec, positions_vec, positions_vec>, image> marker(g, unlimited, marker_lambda);
    make_edge(features_joiner, marker);


    auto inverser_lambda = [] (const image& img) {
        image new_img = img;
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                new_img[i][j] = 255 - new_img[i][j];
            }
        }

        return new_img;
    };

    function_node<image, continue_msg> inverser(g, unlimited, inverser_lambda);
    make_edge(marker, inverser);

    auto mean_calcer_lambda = [&opt](const image& img) {
        double sum = 0;
        for (int i = 0; i < M; ++i)
        {
            for (int j = 0; j < N; ++j)
            {
                sum += img[i][j];
            }
        }

        ofstream file;
        file.open(opt.output_file_path, std::ios::out | std::ios::app);
        if (file.fail())
            std::cout << "Can not open file " + opt.output_file_path << std::endl;

        file << sum / (M * N) << std::endl;
    };

    function_node<image, continue_msg> mean_calcer(g, 1, mean_calcer_lambda);
    make_edge(marker, mean_calcer);

    img_generator.activate();
    g.wait_for_all();

    return 0;
}

