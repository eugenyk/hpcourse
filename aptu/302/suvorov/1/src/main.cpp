#include <assert.h>
#include <pthread.h>
#include <iostream>
#include <stdexcept>
#include <memory>
#include <vector>

struct NoMoreValuesError : std::runtime_error {
  NoMoreValuesError() : std::runtime_error("Value class was closed") {}
};

class PthreadScopedLock {
public:
  PthreadScopedLock(pthread_mutex_t &mutex) : _mutex(mutex) {
    pthread_mutex_lock(&_mutex);
  }
  ~PthreadScopedLock() {
    pthread_mutex_unlock(&_mutex);
  }

private:
  pthread_mutex_t &_mutex;
};

// Acts like a barrier: one thread should call update(), another one 
// should call get(); then both threads will proceed.
class Value {
public:
  Value() : _value(0), _available(false), _is_closed(false) {
    pthread_mutex_init(&_mutex, NULL);
    pthread_cond_init(&_cond, NULL);
  }

  ~Value() {
    pthread_cond_destroy(&_cond);
    pthread_mutex_destroy(&_mutex);
  }

  void update(int value) {
    PthreadScopedLock l(_mutex);
    assert(!_available);

    _value = value;
    _available = true;
    pthread_cond_signal(&_cond);
    while (_available) {
      pthread_cond_wait(&_cond, &_mutex);
    }
  }

  void close() {
    PthreadScopedLock l(_mutex);
    assert(!_available);
    _is_closed = true;
    pthread_cond_signal(&_cond);
  }

  int get() {
    PthreadScopedLock l(_mutex);
    while (!_available && !_is_closed) {
      pthread_cond_wait(&_cond, &_mutex);
    }
    if (!_available) {
      assert(_is_closed);
      throw NoMoreValuesError();
    }
    _available = false;
    pthread_cond_signal(&_cond);
    return _value;
  }
 
private:
  int _value;

  pthread_mutex_t _mutex;
  pthread_cond_t _cond;
  bool _available;
  bool _is_closed;
};

struct ProducerArg {
  std::vector<int> data;
  Value *value;
};
 
void* producer_routine(void* arg_) {
  ProducerArg *arg = static_cast<ProducerArg*>(arg_);
  // Wait for consumer to start: TODO
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  for (int x : arg->data) {
    arg->value->update(x);
  }
  arg->value->close();
  return NULL;
}
 
void* consumer_routine(void* arg) {
  // No race condition, as there were no cancellation points before.
  // TODO: is thread start a cancellation point itself?
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  // notify about start: TODO
  // allocate value for result
  std::unique_ptr<int> result(new int());
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
  Value *value = static_cast<Value*>(arg);
  for (;;) {
    try {
      *result += value->get();
    } catch (NoMoreValuesError&) {
      break;
    }
  }
  return result.release();
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start: TODO
  // interrupt consumer while consumer is running
  // NB: while _consumer_ is running, not producer.
  pthread_t consumer = *static_cast<pthread_t*>(arg);
  while (pthread_kill(consumer, 0) == 0) {
    int result = pthread_cancel(consumer);
    if (result == ESRCH) {
      std::cerr << "Consumer was killed for sure" << "\n";
      break;
    }
    assert(result == 0);
  }
  return NULL;
}
 
int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
  Value v;
  ProducerArg producer_arg;
  pthread_t producer, consumer, consumer_interruptor;

  for (int i = 0; i < 100000; i++) {
    producer_arg.data.push_back(i + 1);
    producer_arg.data.push_back(-i);
  }
  producer_arg.value = &v;
  pthread_create(&producer, NULL, producer_routine, &producer_arg);
  pthread_create(&consumer, NULL, consumer_routine, &v);
  pthread_create(&consumer_interruptor, NULL, consumer_interruptor_routine, &consumer);

  // Should be joined before consumer to avoid consumer's pthread_id reuse.
  pthread_join(consumer_interruptor, NULL);

  void *consumer_return;
  pthread_join(consumer, &consumer_return);
  std::unique_ptr<int> result(static_cast<int*>(consumer_return));
  consumer_return = nullptr;

  pthread_join(producer, NULL);

  return *result;
}
 
int main() {
  std::cout << run_threads() << std::endl;
  return 0;
}
