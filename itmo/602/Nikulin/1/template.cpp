#include <pthread.h>
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

pthread_cond_t shared_cond;
pthread_cond_t cond_consumer, cond_producer, cond_interruptor;
pthread_mutex_t mutex;

int amount_active_worker_threads;
int amount_expected_worker_threads;
unsigned int max_sleep_usecs;
bool alive, empty;

std::vector<pthread_t> consumers;

class mutex_guard {
  public:
    mutex_guard(pthread_mutex_t & mutex) : mutex_(mutex) {
      pthread_mutex_lock(&mutex_);
    }
    ~mutex_guard() {
      pthread_mutex_unlock(&mutex_);
    }
  private:
    pthread_mutex_t & mutex_;
};


struct Value {
  public:
    Value() : _value(0) { }

    void update(int value) {
      _value = value;
    }

    int get() const {
      return _value;
    }

  private:
    int _value;
};


void wait_other() {
  mutex_guard guard(mutex);

#ifdef DEBUG
  printf("amount_active_workers = %d / %d\n",
      amount_active_worker_threads,
      amount_expected_worker_threads);
#endif
  ++amount_active_worker_threads;
  bool need_notify = true;
  while (amount_active_worker_threads < amount_expected_worker_threads && alive) {
    need_notify = false;
    pthread_cond_wait(&shared_cond, &mutex);
  }
  if (need_notify) {
    pthread_cond_broadcast(&shared_cond);
  }
#ifdef DEBUG
  printf("exit from wait\n");
#endif
}


void* producer_routine(void* arg) {
  wait_other();

  Value & value = *(Value*)arg;
  int number;
  while (alive) {
    mutex_guard guard(mutex);
    if (std::cin >> number) {
      value.update(number);
      empty = false;
      pthread_cond_signal(&cond_consumer);
    } else {
      alive = false;
      pthread_cond_broadcast(&cond_consumer);
    }
    while (!empty && alive) {
      pthread_cond_wait(&cond_producer, &mutex);
    }
  }
  return NULL;
}


void* consumer_routine(void* arg) {
  int s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  if (s != 0) {
    printf("%s, \n", "can't set cancel state");
  }
  wait_other();

  Value & value = *(Value*)arg;
  long long sum = 0;
  while (1) {
    {
      mutex_guard guard(mutex);
      while (empty && alive) {
        pthread_cond_wait(&cond_consumer, &mutex);
      }
      if (!alive) {
        break;
      }
      int l = value.get();
      empty = true;
      sum += l;
      pthread_cond_signal(&cond_producer);
    }
    const useconds_t usecs = rand() % max_sleep_usecs;
    usleep(usecs);
  }
#ifdef DEBUG
  printf("consumer exit: %lld\n", sum);
#endif
  return new long long(sum);
}


void* consumer_interruptor_routine(void* arg) {
  wait_other();

  std::vector<pthread_t> const & consumers = *(std::vector<pthread_t>*)arg;
  while (1) {
    for (size_t i = 0; i < consumers.size(); ++i) {
      pthread_cancel(consumers[i]);
    }
    pthread_mutex_lock(&mutex);
    bool local_alive = alive;
    pthread_mutex_unlock(&mutex);
    if (!local_alive) {
      break;
    }
    const useconds_t usecs = rand() % max_sleep_usecs;
    usleep(usecs);
  }
  return NULL;
}


int run_threads(Value const & value, int amount_consumers) {
  pthread_cond_init(&shared_cond, NULL);
  pthread_cond_init(&cond_consumer, NULL);
  pthread_cond_init(&cond_producer, NULL);
  pthread_mutex_init(&mutex, NULL);
  alive = true;
  empty = true;
  amount_active_worker_threads = 0;
  amount_expected_worker_threads = 2 + amount_consumers;

  consumers.resize(amount_consumers);
  for (size_t i = 0; i < consumers.size(); ++i) {
    pthread_create(&consumers[i], NULL, consumer_routine, (void*)&value);
  }

  // Create the threads
  pthread_t producer;
  pthread_t interruptor;
  pthread_create(&producer, NULL, producer_routine, (void*)&value);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, (void*)&consumers);

  pthread_join(producer, NULL);
  pthread_join(interruptor, NULL);

  long long acc_result = 0;
  for (size_t i = 0; i < consumers.size(); ++i) {
    long long * local_result;
    pthread_join(consumers[i], (void**)&local_result);
    acc_result += *local_result;
    delete local_result;
  }
  int ret_value = acc_result;

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond_consumer);
  pthread_cond_destroy(&cond_producer);
  pthread_cond_destroy(&shared_cond);

  return ret_value;
}


int main(int argc, char** argv) {
  if (argc != 3) {
    printf("expected usages: <program> <amount consumers> <upper bound consumers sleep in usecs>");
  }
  Value v;
  const int amount_consumers = std::atoi(argv[1]);
  max_sleep_usecs = std::abs(std::atoi(argv[2]));

  int result = run_threads(v, amount_consumers);
  printf("%d\n", result);
  return 0;
}

