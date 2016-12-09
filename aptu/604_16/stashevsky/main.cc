#include <iostream>
#include <string>
#include <fstream>

#include "graph.h"
#include "utils.h"

using namespace std;

struct Arguments {
    Image::pixel user_value;
    size_t max_images;
    string filename;
};

Arguments ParseArguments(int argc, char **argv) {
    Arguments result;
    for (size_t i = 0; i < argc; ++i) {
        string arg(argv[i]);

        if (arg == "-b") {
            result.user_value = atoi(argv[i + 1]);
        }
        if (arg == "-l") {
            result.max_images = atoi(argv[i + 1]);
        }
        if (arg == "-f") {
            result.filename = argv[i + 1];
        }
    }

    return result;
}

int main(int argc, char **argv) {
    auto arguments = ParseArguments(argc, argv);
    auto result = Measure(128, 128, 10, arguments.max_images);

    if (arguments.filename.size() == 0) {
        return 0;
    }

    ofstream file(arguments.filename);
    for (auto i : result) {
        file << i << " ";
    }
    return 0;
}