#ifndef UNTITLED_CONSUMER_PARAMETERS_H
#define UNTITLED_CONSUMER_PARAMETERS_H
#include "value_storage.h"

struct consumer_parameters {
        value_storage& value;
        const int max_sleep_time;
        const long consumers_number;

        consumer_parameters(class value_storage& value, int max_sleep_time, long consumers_number)
            : value(value), max_sleep_time(max_sleep_time), consumers_number(consumers_number) {}
};

#endif //UNTITLED_CONSUMER_PARAMETERS_H
