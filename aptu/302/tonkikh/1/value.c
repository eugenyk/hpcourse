#include "value.h"

typedef struct value_state_internal {
  int value;  // GUARDED_BY(mutex)
  bool has_value;  // GUARDED_BY(mutex)
  bool consumer_started;  // GUARDED_BY(mutex)
  atomic_bool producer_finished;
  pthread_mutexattr_t mutexattr;
} state_t;

void value_init(value_t* value) {
  state_t* state = malloc(sizeof(state_t));
  *state = (state_t) {
      .value = 0,
      .has_value = false,
      .consumer_started = false,
  };
  atomic_init(&state->producer_finished, false);
  pthread_mutexattr_init(&state->mutexattr);
  pthread_mutexattr_settype(&state->mutexattr, PTHREAD_MUTEX_RECURSIVE);

  *value = (value_t) {
      .consumer = 0,
      .cond = PTHREAD_COND_INITIALIZER,
      .state_ = state,
  };
  pthread_mutex_init(&value->mutex, &state->mutexattr);
}

void value_update(value_t* value, int new_value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  value->state_->value = new_value;
  value->state_->has_value = true;
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

bool value_consume(value_t* value, int* result) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  bool res = false;
  if (value->state_->has_value) {
    *result = value->state_->value;
    value->state_->has_value = false;
    res = true;
  }
  assert_zero(pthread_mutex_unlock(&value->mutex));
  return res;
}

bool value_present(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  bool res = value->state_->has_value;
  assert_zero(pthread_mutex_unlock(&value->mutex));
  return res;
}

void value_wait_consumer(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  while (!value->state_->consumer_started) {
    assert_zero(pthread_cond_wait(&value->cond, &value->mutex));
  }
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

void value_register_consumer(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  value->state_->consumer_started = true;
  value->consumer = pthread_self();
  assert_zero(pthread_cond_broadcast(&value->cond));
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

void value_producer_finish(value_t* value) {
  assert_zero(pthread_mutex_lock(&value->mutex));
  atomic_store(&value->state_->producer_finished, true);
  assert_zero(pthread_cond_broadcast(&value->cond));
  assert_zero(pthread_mutex_unlock(&value->mutex));
}

bool value_producer_finished(value_t* value) {
  return atomic_load(&value->state_->producer_finished);
}
