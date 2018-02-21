#include <pthread.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdatomic.h>
#include <assert.h>
#include <unistd.h>

typedef struct {
  int _value;  // GUARDED_BY(mutex)
  bool _has_value;  // GUARDED_BY(mutex)
  bool _consumer_started;  // GUARDED_BY(mutex)
  atomic_bool producer_finished;
  pthread_t consumer;  // GUARDED_BY(mutex)
  pthread_mutex_t mutex;
  pthread_cond_t cond;  // GUARDED_BY(mutex)
} value_t;
#define VALUE_INIT { \
    ._value = 0, \
    ._has_value = false, \
    ._consumer_started = false, \
    .producer_finished = ATOMIC_VAR_INIT(false), \
    .consumer = 0, \
    .mutex = PTHREAD_MUTEX_INITIALIZER, \
    .cond = PTHREAD_COND_INITIALIZER, \
  }

#define ASSERT_ZERO(cmd) do { int __err = cmd; assert(__err == 0); } while(0)

#ifdef LOG_ENABLE
  #define LOG(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while(0)
#else
  #define LOG(...)
#endif

// REQUIRE(value->mutex)
void value_update(value_t* value, int new_value) {
  value->_value = new_value;
  value->_has_value = true;
}

// REQUIRE(value->mutex)
bool value_consume(value_t* value, int* result) {
  if (value->_has_value) {
    *result = value->_value;
    value->_has_value = false;
    return true;
  } else {
    return false;
  }
}

// REQUIRE(value->mutex)
bool value_present(value_t* value) {
  return value->_has_value;
}

// ACQUIRE(value->mutex)
void value_wait_consumer(value_t* value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
  while (!value->_consumer_started) {
    ASSERT_ZERO(pthread_cond_wait(&value->cond, &value->mutex));
  }
  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

// ACQUIRE(value->mutex)
void value_register_consumer(value_t *value) {
  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
  value->_consumer_started = true;
  value->consumer = pthread_self();
  ASSERT_ZERO(pthread_cond_broadcast(&value->cond));
  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
}

void* producer_routine(void* arg) {
  LOG("[Producer] Waiting for consumer to start");

  value_t* value = (value_t*) arg;
  value_wait_consumer(value);

  LOG("[Producer] Started");

  int x;
  while (true) {
    int scanf_res = scanf("%d", &x);
    if (scanf_res == EOF) {
      break;
    } else if (scanf_res != 1) {
      perror("Error while reading number from standard input");
      exit(scanf_res);
    }

    ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
    value_update(value, x);
    ASSERT_ZERO(pthread_cond_signal(&value->cond));
    while (value_present(value)) {
      ASSERT_ZERO(pthread_cond_wait(&value->cond, &value->mutex));
    }
    ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
  }

  ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
  value->producer_finished = true;
  ASSERT_ZERO(pthread_cond_broadcast(&value->cond));
  ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));

  LOG("[Producer] Finished");
  return NULL;
}

void* consumer_routine(void* arg) {
  LOG("[Consumer] Started");

  value_t* value = (value_t*) arg;
  value_register_consumer(value);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  int* sum = calloc(1, sizeof(*sum));
  while (!value->producer_finished) {
    ASSERT_ZERO(pthread_mutex_lock(&value->mutex));
    int cur = 0;
    while (!value->producer_finished && !value_consume(value, &cur)) {
      ASSERT_ZERO(pthread_cond_wait(&value->cond, &value->mutex));
    }
    ASSERT_ZERO(pthread_cond_signal(&value->cond));
    ASSERT_ZERO(pthread_mutex_unlock(&value->mutex));
    *sum += cur;
  }

  LOG("[Consumer] Finished");
  return sum;
}

void* consumer_interruptor_routine(void* arg) {
  LOG("[Interruptor] Waiting for consumer to start");

  value_t* value = (value_t*) arg;
  value_wait_consumer(value);

  LOG("[Interruptor] Started");

  while (!value->producer_finished) {
    ASSERT_ZERO(pthread_cancel(value->consumer));
  }

  LOG("[Interruptor] Finished");
  return NULL;
}

void check_err(int error_code, const char* error_message) {
  if (error_code != 0) {
    perror(error_message);
    exit(error_code);
  }
}

int run_threads() {
  value_t value = VALUE_INIT;

  pthread_t producer, consumer, interruptor;
  check_err(pthread_create(&producer, NULL, producer_routine, &value),
            "Error creating producer thread\n");
  check_err(pthread_create(&consumer, NULL, consumer_routine, &value),
            "Error creating consumer thread\n");
  check_err(pthread_create(&interruptor, NULL, consumer_interruptor_routine, &value),
            "Error creating interruptor thread\n");

  int* res;
  ASSERT_ZERO(pthread_join(producer, NULL));
  ASSERT_ZERO(pthread_join(consumer, (void**) &res));
  ASSERT_ZERO(pthread_join(interruptor, NULL));

  return *res;
}

int main() {
  printf("%d\n", run_threads());
  return 0;
}
