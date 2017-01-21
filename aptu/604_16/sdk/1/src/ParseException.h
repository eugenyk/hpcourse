//
// Created by dsavvinov on 09.12.16.
//

#ifndef TBB_PARSEEXCEPTION_H
#define TBB_PARSEEXCEPTION_H


#include <stdexcept>

class ParseException : public std::runtime_error {
public:
    ParseException(std::string const & message)
        : std::runtime_error(message)
    { }
};


#endif //TBB_PARSEEXCEPTION_H
