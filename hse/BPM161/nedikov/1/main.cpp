#include <iostream>
#include <pthread.h>
#include <sstream>
#include <random>

#ifndef UNIX
  #include <pthread_time.h>
#endif

struct thread_data_t {
  thread_data_t(int millis, int n, pthread_t *consumers)
      : consumers(consumers), n(n) {
    sleep_time.tv_sec = 0;
    sleep_time.tv_nsec = millis * 1000;

    pthread_barrier_init(&barrier, nullptr, (unsigned) n + 2);
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_mutex_init(&cons_mutex, nullptr);
    pthread_cond_init(&cons_cond, nullptr);
  }

  ~thread_data_t() {
    pthread_barrier_destroy(&barrier);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&cons_mutex);
    pthread_cond_destroy(&cons_cond);
  }

  int n;
  pthread_t *consumers;

  pthread_barrier_t barrier;
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_mutex_t cons_mutex;
  pthread_cond_t cons_cond;

  timespec sleep_time{};

  bool working = true;
  bool has_value = false;
  int value = 0;
};

struct interrupter_data_t {
  interrupter_data_t(int n, pthread_t *consumers, pthread_barrier_t *barrier)
      : n(n), consumers(consumers), barrier(barrier) {}

  int n;
  pthread_t *consumers;
  pthread_barrier_t *barrier;
};

void *producer_routine(void *arg) {
  auto data = static_cast<thread_data_t *>(arg);
  pthread_barrier_wait(&data->barrier);

  std::string str;
  std::getline(std::cin, str);
  std::stringstream sin(str);

  int k;
  while (sin >> k) {
    pthread_mutex_lock(&data->mutex);
    while (data->has_value) {
      // std::cerr << "wait cons_cond" << std::endl;
      pthread_cond_wait(&data->cons_cond, &data->mutex);
    }

    data->value = k;
    data->has_value = true;
    // std::cerr << "cond signal" << std::endl;
    pthread_cond_signal(&data->cond);
    pthread_mutex_unlock(&data->mutex);
  }

  pthread_mutex_lock(&data->mutex);
  while (data->has_value) {
    // std::cerr << "wait final cons_cond" << std::endl;
    pthread_cond_wait(&data->cons_cond, &data->mutex);
  }
  pthread_mutex_unlock(&data->mutex);

  data->value = 0;
  data->working = false;
  // std::cerr << "broadcast" << std::endl;
  pthread_cond_broadcast(&data->cond);
}

void *consumer_routine(void *arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  int *sum = new int(0);
  auto data = static_cast<thread_data_t *>(arg);
  pthread_barrier_wait(&data->barrier);

  while (data->working) {
    pthread_mutex_lock(&data->mutex);
      while (data->working && !data->has_value) {
        // std::cerr << "wait cond" << std::endl;
        pthread_cond_wait(&data->cond, &data->mutex);
      }
      int k = data->value;
      data->has_value = false;
      // std::cerr << "cons_cond signal" << std::endl;
      pthread_cond_signal(&data->cons_cond);
      // std::cerr << "consumed " << k << std::endl;
      *sum += k;
    pthread_mutex_unlock(&data->mutex);
    nanosleep(&data->sleep_time, &data->sleep_time);
  }
  return sum;
}

void *consumer_interruptor_routine(void *arg) {
  auto data = static_cast<thread_data_t *>(arg);
  pthread_barrier_wait(&data->barrier);

  while (data->working) {
    int i = std::rand() % data->n;
    pthread_cancel(data->consumers[i]);
  }
}

int run_threads(int n, int millis) {
  pthread_t producer;
  pthread_t interrupter;
  auto *consumers = new pthread_t[n];

  thread_data_t data(millis, n, consumers);

  pthread_create(&producer, nullptr, producer_routine, &data);
  pthread_create(&interrupter, nullptr, consumer_interruptor_routine, &data);

  for (int i = 0; i < n; i++) {
    pthread_create(&consumers[i], nullptr, consumer_routine, &data);
  }

  int sum = 0;
  pthread_join(producer, nullptr);
  pthread_join(interrupter, nullptr);
  for (int i = 0; i < n; i++) {
    int *res = nullptr;
    pthread_join(consumers[i], (void **) &res);
    sum += *res;
    delete res;
  }

  delete[](consumers);

  return sum;
}

int main(int argc, char **argv) {
  std::cout << run_threads(atoi(argv[1]), atoi(argv[2])) << std::endl;
  return 0;
}