#ifndef ARGSPARSER_H
#define ARGSPARSER_H

#include <stdexcept>
#include "structs.h"

class ArgsParser
{
private:
    ArgsParser();

public:
    static bool parse(int argc, char **argv, graph_options& opt);

};

#endif // ARGSPARSER_H
