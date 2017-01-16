#ifndef MathUtils_h
#define MathUtils_h

#include <iostream>

namespace labutils {
    int random(int min, int max) {
        return rand() % max + min;
    }
}

#endif /* MathUtils_h */
