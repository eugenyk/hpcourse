#include <pthread.h>
#include <iostream>

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t value_cond = PTHREAD_COND_INITIALIZER;

// TODO: enum
volatile int state = 0; // 0 - consumed, 1 - produced, 3 - terminated

class Value {
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
 
void* producer_routine(void* arg) {
  Value *value = (Value*)arg;
  pthread_mutex_lock(&value_mutex);
  
  // Wait for consumer to start
  while (state != 1) {
    pthread_cond_wait(&value_cond, &value_mutex);
  }


  int x;
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  while (std::cin >> x) {
    value->update(x);
 
    state = 0;
    pthread_cond_broadcast(&value_cond);
 
    while (state != 1) {
      pthread_cond_wait(&value_cond, &value_mutex);
    }
  }

  state = 3;
  pthread_cond_broadcast(&value_cond);
 
  pthread_mutex_unlock(&value_mutex);
}
 
void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  Value *value = (Value*)arg;

  pthread_mutex_lock(&value_mutex);
  
  // allocate value for result
  int* result = new int(0);
  while (true) {
 
    // notify about start
    state = 1;
    pthread_cond_broadcast(&value_cond);
 
    while (state == 1) {
      pthread_cond_wait(&value_cond, &value_mutex);
    }
    if (state == 3) {
      break;
    }
    // for every update issued by producer, read the value and add to sum
    *result += value->get();
  }

  pthread_mutex_unlock(&value_mutex);

  // return pointer to result
  return result;
}
 
void* consumer_interruptor_routine(void* arg) {
  pthread_t *consumer_thread = (pthread_t*) arg;

  while (true) {
    // wait for consumer to start
    if (state == 1) {
      // interrupt consumer while producer is running                                          
      pthread_cancel(*consumer_thread);
    }
    if (state == 3) {
      break;
    }
  }
}
 
int run_threads() {
  // start 3 threads and wait until they're done

  Value value;

  pthread_t consumer_thread;
  pthread_create(&consumer_thread, NULL, consumer_routine, (void*)(&value));

  pthread_t producer_thread;
  pthread_create(&producer_thread, NULL, producer_routine, (void*)(&value));


  pthread_t consumer_interruptor_thread;
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, (void*)(&consumer_thread));

  int *result;
  pthread_join(consumer_thread, (void**)(&result));
  pthread_join(producer_thread, NULL);
  pthread_join(consumer_interruptor_thread, NULL);

  // return sum of update values seen by consumer
  return *result;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}

