#include <pthread.h>
#include <iostream>
#include <sstream>

class ReadOnceValue {
public:
  ReadOnceValue() : _value(0), _valid(false) {
  }

  void refresh(int value) {
    _value = value;
    _valid = true;
  }

  int extract() {
    if (!isValid()) {
      throw std::logic_error("ReadOnceValue is requested the second time!");
    }
    _valid = false;
    return _value;
  }

  bool isValid() {
    return _valid;
  }

private:
  volatile int _value;
  volatile bool _valid;
};

pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumer_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

volatile bool producing_finished = false;
volatile bool consuming_started = false;

void* producer_routine(void* arg) {
  pthread_mutex_lock(&consumer_mutex);
  while (!consuming_started) {
    pthread_cond_wait(&consumer_cond, &consumer_mutex);
  }
  pthread_mutex_unlock(&consumer_mutex);

  auto value = (ReadOnceValue*) arg;
  std::string input;
  std::getline(std::cin, input);
  std::istringstream iss(input);
  int x;
  pthread_mutex_lock(&consumer_mutex);
  while (iss >> x) {
    pthread_mutex_lock(&producer_mutex);
    value->refresh(x);
    pthread_cond_broadcast(&producer_cond);
    pthread_mutex_unlock(&producer_mutex);

    while (value->isValid()) {
      pthread_cond_wait(&consumer_cond, &consumer_mutex);
    }
  }
  pthread_mutex_unlock(&consumer_mutex);

  pthread_mutex_lock(&producer_mutex);
  producing_finished = true;
  pthread_cond_broadcast(&producer_cond);
  pthread_mutex_unlock(&producer_mutex);

  pthread_exit(nullptr);
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
  auto value = (ReadOnceValue*) arg;
  int result = 0;
  pthread_mutex_lock(&producer_mutex);

  pthread_mutex_lock(&consumer_mutex);
  consuming_started = true;
  pthread_cond_broadcast(&consumer_cond);
  pthread_mutex_unlock(&consumer_mutex);

  while (true) {
    while (!value->isValid() && !producing_finished) {
      pthread_cond_wait(&producer_cond, &producer_mutex);
    }
    if (producing_finished) {
      break;
    }
    result += value->extract();

    pthread_mutex_lock(&consumer_mutex);
    pthread_cond_broadcast(&consumer_cond);
    pthread_mutex_unlock(&consumer_mutex);
  }
  pthread_mutex_unlock(&producer_mutex);
  pthread_exit(reinterpret_cast<void *>(result));
}

void* consumer_interruptor_routine(void* arg) {
  pthread_mutex_lock(&consumer_mutex);
  while (!consuming_started) {
    pthread_cond_wait(&consumer_cond, &consumer_mutex);
  }
  pthread_mutex_unlock(&consumer_mutex);

  auto consumer_thread = (pthread_t*) arg;
  while (!producing_finished) {
    pthread_cancel(*consumer_thread);
  }
  pthread_exit(nullptr);
}

void __ensure_pthread_created(int return_code) {
  if (return_code != 0) {
    std::cout << "ERROR while creating pthread; error code " << return_code << std::endl;
    exit(-1);
  }
}

int run_threads() {
  auto value = std::make_shared<ReadOnceValue>();

  pthread_t producer_thread;
  int return_code = pthread_create(&producer_thread, nullptr, producer_routine, value.get());
  __ensure_pthread_created(return_code);

  pthread_t consumer_thread;
  return_code = pthread_create(&consumer_thread, nullptr, consumer_routine, value.get());
  __ensure_pthread_created(return_code);

  pthread_t consumer_interruptor_thread;
  return_code = pthread_create(
      &consumer_interruptor_thread, nullptr, consumer_interruptor_routine, &consumer_thread);
  __ensure_pthread_created(return_code);

  pthread_join(producer_thread, nullptr);
  int result;
  pthread_join(consumer_thread, reinterpret_cast<void **>(&result));
  pthread_join(consumer_interruptor_thread, nullptr);
  return result;
}

int main() {
  std::cout << run_threads() << std::endl;
  return 0;
}
