#ifndef BASE_H
#define BASE_H
#include <string>
#include <stdexcept>
#include <iostream>
using namespace std;

struct base {
    int height;
    int width;
    int spec; 
    int imagSizeMax;
    bool logData;
    string fileName;
};
bool parse(int argc, char** argv, base& args);
#endif