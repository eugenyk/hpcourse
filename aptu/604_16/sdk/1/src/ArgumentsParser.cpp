//
// Created by dsavvinov on 09.12.16.
//

#include "ArgumentsParser.h"
#include "ParseException.h"

void ArgumentsParser::parse(int argc, char **argv) {
    using namespace std;

    int i = 1;
    bool hasBrighntess, hasLog, hasLimit;
    hasBrighntess = hasLog = hasLimit = 0;

    while (i < argc) {
        if (argv[i] == string("-b")) {
            if (i + 1 >= argc) {
                throw ParseException("Error: brightness value was expected, but end of arglist was found");
            }
            brightness = std::atoi(argv[i + 1]);
            hasBrighntess = true;
        } else if (argv[i] == string("-f")) {
            if (i + 1 >= argc) {
                throw ParseException("Error: log-file name was expected, but end of arglist was found");
            }
            log = argv[i + 1];
            hasLog = true;
        } else if (argv[i] == string("-l")) {
            if (i + 1 >= argc) {
                throw ParseException("Error: limit was expected, but end of arglist was found");
            }
            limit = std::atoi(argv[i + 1]);
            hasLimit = true;
        } else {
            throw ParseException(std::string("Error: unknown option ") + std::string(argv[i]));
        }
        i += 2;
    }

    if (!hasBrighntess) {
        throw ParseException("Error: brightness value not found");
    }

    if (!hasLog) {
        throw ParseException("Error: log-file name not found");
    }

    if (!hasLimit) {
        throw ParseException("Error: limit not found");
    }

    return;
}

int ArgumentsParser::getBrightnessOption() {
    return brightness;
}

int ArgumentsParser::getParallelismLimit() {
    return limit;
}

std::string ArgumentsParser::getLogFileName() {
    return log;
}
