#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

class Utils
{
public:
    Utils();
    ~Utils();

    static std::tuple<int, int, int, std::string > argsProcessing(int argc, char* argv[]);
};

#endif // UTILS_H
