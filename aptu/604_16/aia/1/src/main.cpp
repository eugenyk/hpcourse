#include <cstdlib>
#include <iostream>

#include "random_image.h"
#include "input_parser.h"


int main(int argc, char** argv) {
    using namespace std;
    using namespace random_image;
    using namespace input_parser;
    
    InputParser parser(argc, argv);
    if (parser.isValid())
    {
        std::cout << parser.toString() << std::endl;
    }
    else
    {
        std::cout << __func__ << ": input parameters are invalid!" << std::endl;
        std::cout << "HOWTO: -b <brightness [0, 255]>  -l <task limit> -f <output file path>" << endl;
        return -1;
    }
    
    return 0;
}

