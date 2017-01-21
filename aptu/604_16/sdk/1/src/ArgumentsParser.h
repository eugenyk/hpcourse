//
// Created by dsavvinov on 09.12.16.
//

#ifndef TBB_ARGUMENTSPARSER_H
#define TBB_ARGUMENTSPARSER_H

#include <string>

class ArgumentsParser {
public:
    void parse(int argc, char ** argv);
    int getBrightnessOption();
    int getParallelismLimit();
    std::string getLogFileName();
private:
    int brightness;
    int limit;
    std::string log;
};


#endif //TBB_ARGUMENTSPARSER_H
