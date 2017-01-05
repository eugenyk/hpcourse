#include <iostream>
#include <string>
#include <fstream>

#include "graph.h"
#include "utils.h"

using namespace std;

struct Arguments {
    int user_value = -1;
    size_t max_images = 1;
    string filename = "output.txt";
};

Arguments ParseArguments(int argc, char **argv) {
    Arguments result = Arguments();
    for (size_t i = 0; i < argc; ++i) {
        string arg(argv[i]);

        if (arg == "-b") {
            result.user_value = atoi(argv[i + 1]);
        } else
        if (arg == "-l") {
            result.max_images = static_cast<size_t>(atoi(argv[i + 1]));
        } else
        if (arg == "-f") {
            result.filename = argv[i + 1];
        }
    }

    return result;
}

int main(int argc, char **argv) {
    auto arguments = ParseArguments(argc, argv);
    auto result = Measure(1280, 1280, 10000, arguments.user_value, arguments.max_images);

    if (arguments.filename.size() == 0) {
        return 0;
    }

    ofstream file(arguments.filename);
    for (auto i : result) {
        file << i << " ";
    }
    return 0;
}