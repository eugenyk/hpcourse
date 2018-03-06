#ifndef FIRST_ASSIGNMENT_VALUE_H
#define FIRST_ASSIGNMENT_VALUE_H

#include <pthread.h>
#include <stdbool.h>
#include "utils.h"

typedef struct value_internal value_t;


value_t* create_value();

void free_value(value_t* value);

void value_produce(value_t* value, int new_value);

void value_wait_until_consumed(value_t* value);

bool value_consume(value_t* value, int* consumed);

void value_wait_consumer(value_t* value);

void value_consumer_start(value_t* value);

void value_producer_finish(value_t* value);

bool value_producer_finished(value_t* value);

#endif  // FIRST_ASSIGNMENT_VALUE_H
