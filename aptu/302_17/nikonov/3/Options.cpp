//
// Created by michael on 22.05.17.
//

#include <exception>
#include <stdexcept>
#include "Options.h"

Options::Options(u_char _brightness, size_t _limit, std::string _log_name)
        : brightness(_brightness), limit(_limit), log_name(_log_name) { }

Options::Options(int argc, char** argv) {
    uint args_mask = 0;
    log_name = "";
    if (!(argc % 2)) {
        throw std::invalid_argument("Invalid number of arguments");
    }
    for (int i = 1; i < argc; i += 2) {
        std::string arg = std::string(argv[i]);
        if (arg == "-b") {
            args_mask |= BFLAG;
            brightness = (u_char) atoi(argv[i + 1]);
        } else if (arg == "-l") {
            args_mask |= LIMIT_FLAG;
            limit = (u_char) atoi(argv[i + 1]);
        } else if (arg == "-f") {
            args_mask |= LOG_FLAG;
            log_name = std::string(argv[i + 1]);
        }
    }
    if (!(args_mask & BFLAG) || !(args_mask & LIMIT_FLAG)) {
        throw std::invalid_argument("Some arguments are not set");
    }
}

