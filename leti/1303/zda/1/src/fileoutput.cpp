#include "fileoutput.h"

FileOutput::FileOutput(std::ofstream & fs) : _fs(fs)
{

}

void FileOutput::operator()(double value)
{
    _fs << value << std::endl;
}
