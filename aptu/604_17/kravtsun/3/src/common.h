#ifndef LAB03_COMMON_H
#define LAB03_COMMON_H


#include <iostream>
#include <mutex>

static std::mutex log_mutex;
#define LOG(message) \
    do {\
        std::unique_lock<std::mutex> log_lock(log_mutex);\
        std::cout << __FILE__ << ":" << __LINE__ << " " << message << std::endl;\
    } while (0);

#endif //LAB03_COMMON_H
