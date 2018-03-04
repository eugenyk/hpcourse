#include <stdatomic.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include "value.h"

struct value_internal {
  pthread_t consumer;  // GUARDED_BY(mutex)

  int value;  // GUARDED_BY(mutex)
  bool has_value;  // GUARDED_BY(mutex)
  bool consumer_started;  // GUARDED_BY(mutex)
  pthread_mutex_t mutex;
  pthread_cond_t cond;  // GUARDED_BY(mutex)
  atomic_bool producer_finished;
};

value_t* create_value() {
  value_t* value = malloc(sizeof(*value));

  *value = (value_t) {
      .mutex = PTHREAD_MUTEX_INITIALIZER,
      .cond = PTHREAD_COND_INITIALIZER,
      .value = 0,
      .has_value = false,
      .consumer_started = false,
  };
  atomic_init(&value->producer_finished, false);

  return value;
}

pthread_t value_consumer(value_t* value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));

  if (!value->consumer_started) {
    int err = ENODATA;
    check_err(err, "Trying to access value consumer while it's not registered yet");
  }
  pthread_t res = value->consumer;

  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
  return res;
}

void value_produce(value_t* value, int new_value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));

  value->value = new_value;
  value->has_value = true;
  ASSERT_ZERO(pthread_cond_signal(&value->cond));

  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

void value_wait_until_consumed(value_t* value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
  while (value->has_value) {
    ASSERT_ZERO(pthread_cond_wait(&value->cond, &value->mutex));
  }
  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

bool value_consume(value_t* value, int* consumed) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));

  while (!value_producer_finished(value) && !value->has_value) {
    ASSERT_ZERO(pthread_cond_wait(&value->cond, &value->mutex));
  }
  bool ret = false;
  if (value->has_value) {
    *consumed = value->value;
    value->has_value = false;
    ret = true;
  }
  ASSERT_ZERO(pthread_cond_signal(&value->cond));

  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
  return ret;
}

void value_wait_consumer(value_t* value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
  while (!value->consumer_started) {
    ASSERT_ZERO(pthread_cond_wait(&value->cond, &value->mutex));
  }
  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

void value_register_consumer(value_t* value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));

  value->consumer_started = true;
  value->consumer = pthread_self();
  ASSERT_ZERO(pthread_cond_broadcast(&value->cond));

  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

void value_producer_finish(value_t* value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));

  atomic_store(&value->producer_finished, true);
  ASSERT_ZERO(pthread_cond_broadcast(&value->cond));

  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

bool value_producer_finished(value_t* value) {
  return atomic_load(&value->producer_finished);
}
