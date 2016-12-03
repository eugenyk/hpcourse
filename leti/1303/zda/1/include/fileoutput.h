#ifndef FILEOUTPUT_H
#define FILEOUTPUT_H

#include "common.h"
#include <iostream>
#include <fstream>
#include <string>

class FileOutput
{
    std::string _filename;
public:
    explicit FileOutput(std::string filename);
    void operator()( double value);
};

#endif // FILEOUTPUT_H
