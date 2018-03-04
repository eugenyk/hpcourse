#include "value.h"

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

    assert_zero(pthread_mutex_lock(&value->mutex));
    value_update(value, x);
    assert_zero(pthread_cond_signal(&value->cond));
    while (value_present(value)) {
      assert_zero(pthread_cond_wait(&value->cond, &value->mutex));
    }
    assert_zero(pthread_mutex_unlock(&value->mutex));
  }

  value_producer_finish(value);
  LOG("[Producer] Finished");
  return NULL;
}

void* consumer_routine(void* arg) {
  LOG("[Consumer] Started");

  value_t* value = (value_t*) arg;
  value_register_consumer(value);
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  int* sum = calloc(1, sizeof(*sum));
  while (!value_producer_finished(value)) {
    assert_zero(pthread_mutex_lock(&value->mutex));
    int cur = 0;
    while (!value_producer_finished(value) && !value_consume(value, &cur)) {
      assert_zero(pthread_cond_wait(&value->cond, &value->mutex));
    }
    assert_zero(pthread_cond_signal(&value->cond));
    assert_zero(pthread_mutex_unlock(&value->mutex));
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

  while (!value_producer_finished(value)) {
    assert_zero(pthread_cancel(value->consumer));
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
  value_t value;
  value_init(&value);

  pthread_t producer, consumer, interruptor;
  check_err(pthread_create(&producer, NULL, producer_routine, &value),
            "Error creating producer thread\n");
  check_err(pthread_create(&consumer, NULL, consumer_routine, &value),
            "Error creating consumer thread\n");
  check_err(pthread_create(&interruptor, NULL, consumer_interruptor_routine, &value),
            "Error creating interruptor thread\n");

  int* res;
  assert_zero(pthread_join(producer, NULL));
  assert_zero(pthread_join(consumer, (void**) &res));
  assert_zero(pthread_join(interruptor, NULL));

  return *res;
}

int main() {
  printf("%d\n", run_threads());
  return 0;
}
