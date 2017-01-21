#include <iostream>
#include <vector>

#include "ImageProcessor.h"

namespace {
    void usage(char const *name) {
        std::cout << "Usage: " << name;
        std::cout << " [-f filename] ";
        std::cout << " [-b value] ";
        std::cout << " [-l number] ";
        std::cout << std::endl << std::endl;
        std::cout << "OPTIONS\n";
        std::cout << "\t-f filename\t Use it to specify a path where program log with average values will be written (default `flow-graph.log`)." << std::endl;
        std::cout << "\t-b value\t Use it to set brightness value that will be searched in image (default `128`)." << std::endl;
        std::cout << "\t-l number\t This option sets number of images processed simultaneously (default `4`)." << std::endl;
    }

    std::vector<Image> create_images(size_t n) {
        std::vector<Image> images;
        for (size_t i = 0; i < n; ++i) {
            images.push_back(Image(512, 512));
        }
        return images;
    }
}

int main(int argc, char **argv) {
    int pixel_to_search = 128;
    size_t parallel_images = 4;
    std::string log_fname = "flow-graph.log";

    for (int i = 1; i < argc; i += 2) {
        std::string flag = argv[i];
        if (flag == "-h" || flag == "--help") {
            usage(argv[0]);
            exit(0);
        } else if (flag == "-f") {
            log_fname = argv[i + 1];
        } else if (flag == "-b") {
            pixel_to_search = std::stoi(argv[i + 1]);
        } else if (flag == "-l") {
            parallel_images = std::stoul(argv[i + 1]);
        } else {
            usage(argv[0]);
            exit(1);
        }
    }

    ImageProcessor ip(create_images(100), (pixel_t) pixel_to_search, parallel_images, log_fname);
    ip.process();

    return 0;
}