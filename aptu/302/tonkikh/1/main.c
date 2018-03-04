#include <stdlib.h>
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

    value_produce(value, x);
    value_wait_until_consumed(value);
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
  int cur = 0;
  while (value_consume(value, &cur)) {
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
    // the consumer might have already finished its execution
    MAY_BE_NONZERO(pthread_cancel(value_consumer_thread(value)));
  }

  LOG("[Interruptor] Finished");
  return NULL;
}

int run_threads() {
  value_t* value = create_value();

  pthread_t producer, consumer, interruptor;
  check_err(pthread_create(&producer, NULL, producer_routine, value),
            "Error creating producer thread\n");
  check_err(pthread_create(&consumer, NULL, consumer_routine, value),
            "Error creating consumer thread\n");
  check_err(pthread_create(&interruptor, NULL, consumer_interruptor_routine, value),
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
