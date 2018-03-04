#ifndef FIRST_ASSIGNMENT_VALUE_H
#define FIRST_ASSIGNMENT_VALUE_H

#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <unistd.h>
#include "utils.h"

/// Should be initialized by `value_init` function
typedef struct {
  int value_;  // GUARDED_BY(mutex)
  bool has_value_;  // GUARDED_BY(mutex)
  bool consumer_started_;  // GUARDED_BY(mutex)
  atomic_bool producer_finished_;
  pthread_t consumer;  // GUARDED_BY(mutex)
  pthread_mutex_t mutex;
  pthread_cond_t cond;  // GUARDED_BY(mutex)
} value_t;

void value_init(value_t* value) {
  *value = (value_t) {
      .value_ = 0,
      .has_value_ = false,
      .consumer_started_ = false,
      .consumer = 0,
      .mutex = PTHREAD_MUTEX_INITIALIZER,
      .cond = PTHREAD_COND_INITIALIZER,
  };
  atomic_init(&value->producer_finished_, false);
}

// REQUIRE(value->mutex)
void __require_mutex_value_update(value_t* value, int new_value) {
  value->value_ = new_value;
  value->has_value_ = true;
}

// REQUIRE(value->mutex)
bool __require_mutex_value_consume(value_t* value, int* result) {
  if (value->has_value_) {
    *result = value->value_;
    value->has_value_ = false;
    return true;
  } else {
    return false;
  }
}

// REQUIRE(value->mutex)
bool __require_mutex_value_present(value_t* value) {
  return value->has_value_;
}

// ACQUIRE(value->mutex)
void __acquire_mutex_value_wait_consumer(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  while (!value->consumer_started_) {
    assert_zero(pthread_cond_wait(&value->cond, &value->mutex));
  }
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

// ACQUIRE(value->mutex)
void __acquire_mutex_value_register_consumer(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  value->consumer_started_ = true;
  value->consumer = pthread_self();
  assert_zero(pthread_cond_broadcast(&value->cond));
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

void value_producer_finish(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  atomic_store(&value->producer_finished_, true);
  assert_zero(pthread_cond_broadcast(&value->cond));
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

bool value_producer_finished(value_t* value) {
  return atomic_load(&value->producer_finished_);
}

#endif  // FIRST_ASSIGNMENT_VALUE_H
