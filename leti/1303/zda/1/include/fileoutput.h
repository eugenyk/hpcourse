#ifndef FILEOUTPUT_H
#define FILEOUTPUT_H

#include "common.h"
#include <iostream>
#include <fstream>
#include <string>

class FileOutput
{
    std::ofstream & _fs;
public:
    explicit FileOutput(std::ofstream &fs);
    void operator()( double value);
};

#endif // FILEOUTPUT_H
