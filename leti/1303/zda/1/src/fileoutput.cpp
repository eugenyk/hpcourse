#include "fileoutput.h"

FileOutput::FileOutput(std::string filename)
{
    _filename = filename;
}

void FileOutput::operator()(double value)
{
    std::ofstream fs;
    fs.open(_filename.c_str(), std::ios::out | std::ios::app);
    fs << value << std::endl;
    fs.close();
}
