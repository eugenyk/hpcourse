#include "argsparser.h"
#include "myflowgraph.h"
#include <iostream>

void print_usage()
{
    std::cout << "Arguments parsing error\n";
    std::cout << "Example:\n"
                 "flow_graph -h 1024 -w 1024 -b 128 -l 5 -f log.txt\n"
                 "-h\theight\n"
                 "-w\twidth\n"
                 "-b\tbrightness\n"
                 "-l\tconcurrent images limit\n"
                 "-f\tlog file name" << std::endl;
}

int main(int argc, char *argv[])
{
    graph_options opt;
    if(!ArgsParser::parse(argc, argv, opt))
    {
        print_usage();
        return -1;
    }

    MyFlowGraph g(opt);
    g.run();

    return 0;
}
