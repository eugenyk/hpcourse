//
// Created by michael on 22.05.17.
//

#ifndef HW03_OPTIONS_H
#define HW03_OPTIONS_H

#include <cstddef>
#include <string>

class Options {
public:
    u_char brightness;
    size_t limit;
    std::string log_name;
    Options(u_char _brightness = 0, size_t _limit = 1, std::string _log_name = "log.txt");
    Options(int argc, char** argv);

private:
    const uint BFLAG = 1;
    const uint LIMIT_FLAG = 2;
    const uint LOG_FLAG = 4;
};

#endif //HW03_OPTIONS_H
