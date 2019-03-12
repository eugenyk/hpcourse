#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

enum shared_value_status {
    AVAILABLE_FOR_WRITE = 0,
    AVAILABLE_FOR_READ
};

struct shared_value {
    pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;
    pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
    shared_value_status status = AVAILABLE_FOR_WRITE;
    int value = 0;
};

static unsigned int CONSUMER_SLEEP_TIME;
static pthread_barrier_t consumers_start_barrier;
static bool producer_done = false;

void *producer_routine(void *arg) {
  shared_value *value = static_cast<shared_value *>(arg);

  // Wait for consumer to start.
  pthread_barrier_wait(&consumers_start_barrier);

  // Read data.
  std::string line;
  std::getline(std::cin, line);

  std::istringstream line_stream(line);
  int number;

  // Loop through each value and update the value.
  while (line_stream >> number) {
    pthread_mutex_lock(&value->value_mutex);

    // Wait for consumer to process.
    while (value->status != AVAILABLE_FOR_WRITE) {
      pthread_cond_wait(&value->can_produce, &value->value_mutex);
    }

    value->value = number;
    value->status = AVAILABLE_FOR_READ;

    pthread_mutex_unlock(&value->value_mutex);

    // Notify consumer.
    pthread_cond_signal(&value->can_consume);
  }

  pthread_mutex_lock(&value->value_mutex);

  // Wait for consumer to process.
  while (value->status != AVAILABLE_FOR_WRITE) {
    pthread_cond_wait(&value->can_produce, &value->value_mutex);
  }

  // Notify that we are done.
  producer_done = true;

  pthread_mutex_unlock(&value->value_mutex);
  pthread_cond_broadcast(&value->can_consume);

  return nullptr;
}

__thread int TLS_aggregated_value = 0;

void random_sleep() {
  if (CONSUMER_SLEEP_TIME != 0) {
    std::this_thread::sleep_for(std::chrono::milliseconds(rand() % CONSUMER_SLEEP_TIME));
  }
}

void *consumer_routine(void *arg) {
  shared_value *value = static_cast<shared_value *>(arg);
  int oldstate;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);

  // Notify about start.
  pthread_barrier_wait(&consumers_start_barrier);

  // For every update issued by producer, read the value and add to sum
  while (!producer_done) {
    pthread_mutex_lock(&value->value_mutex);

    while (value->status != AVAILABLE_FOR_READ && !producer_done) {
      pthread_cond_wait(&value->can_consume, &value->value_mutex);
    }

    if (producer_done) {
      // Not forgetting to unlock mutex.
      pthread_mutex_unlock(&value->value_mutex);
      break;
    }

    TLS_aggregated_value += value->value;
    value->status = AVAILABLE_FOR_WRITE;

    pthread_mutex_unlock(&value->value_mutex);
    pthread_cond_signal(&value->can_produce);

    random_sleep();
  }

  // return pointer to result (for particular consumer)
  return &TLS_aggregated_value;
}

void *consumer_interrupter_routine(void *arg) {
  std::vector<pthread_t> *consumers = static_cast<std::vector<pthread_t> *>(arg);

  // wait for consumers to start
  pthread_barrier_wait(&consumers_start_barrier);

  // interrupt random consumer while producer is running
  if (!consumers->empty()) {
    while (!producer_done) {
      size_t idx = rand() % consumers->size();
      pthread_cancel((*consumers)[idx]);
    }
  }

  return nullptr;
}

int run_threads(size_t num_threads) {
  // start N threads and wait until they're done
  // return aggregated sum of values

  shared_value value;

  pthread_t producer_thread;
  pthread_t interrupter_thread;
  std::vector<pthread_t> consumer_threads(num_threads);

  pthread_barrier_init(&consumers_start_barrier, nullptr,
                       static_cast<unsigned int>(num_threads + 2)); /* number of producers + consumer */

  for (size_t i = 0; i < num_threads; ++i) {
    pthread_create(&(consumer_threads[i]), nullptr, consumer_routine, &value);
  }

  pthread_create(&interrupter_thread, nullptr, consumer_interrupter_routine, &consumer_threads);
  pthread_create(&producer_thread, nullptr, producer_routine, &value);

  pthread_join(producer_thread, nullptr);
  pthread_join(interrupter_thread, nullptr);

  int sum = 0;
  for (size_t i = 0; i < num_threads; ++i) {
    void *val;
    pthread_join(consumer_threads[i], &val);
    sum += *(int *) val;
  }

  return sum;
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Provide 2 arguments" << std::endl;
    return 1;
  }

  CONSUMER_SLEEP_TIME = atoi(argv[2]);
  std::cout << run_threads(static_cast<size_t>(atoi(argv[1]))) << std::endl;

  return 0;
}
