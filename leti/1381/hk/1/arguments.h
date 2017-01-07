#ifndef ARGUMENTS_H
#define ARGUMENTS_H
#include <string>

struct arguments {
    int height;
    int width;
    int spec; //заданная яркость
    int imagesLimit;
    bool logging;
    std::string filename;
};

bool parse(int argc, char** argv, arguments& args);

#endif // ARGUMENTS_H
