#ifndef FIRST_ASSIGNMENT_VALUE_H
#define FIRST_ASSIGNMENT_VALUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>
#include "utils.h"

struct value_state_internal;

/// Should be initialized by `value_init` function
typedef struct {
  pthread_t consumer;  // GUARDED_BY(mutex)
  pthread_mutex_t mutex;
  pthread_cond_t cond;  // GUARDED_BY(mutex)
  struct value_state_internal* state_;
} value_t;

void value_init(value_t* value);

void value_update(value_t* value, int new_value);

bool value_consume(value_t* value, int* result);

bool value_present(value_t* value);

void value_wait_consumer(value_t* value);

void value_register_consumer(value_t* value);

void value_producer_finish(value_t* value);

bool value_producer_finished(value_t* value);

#endif  // FIRST_ASSIGNMENT_VALUE_H
