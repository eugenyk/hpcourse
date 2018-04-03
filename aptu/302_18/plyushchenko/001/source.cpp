#include <pthread.h>
#include <iostream>
 
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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
const char NOT_STARTED = 0;
const char STARTED = 1;
const char PRODUCED = 2;
const char CONSUMED = 3;
const char FINISHED = 4;
volatile char state = NOT_STARTED;
 
inline void wait_for_consumer_to_start() {
  pthread_mutex_lock(&mutex);
  while (state == NOT_STARTED) {
    pthread_cond_wait(&cond, &mutex);
  }	 
  pthread_mutex_unlock(&mutex);
}

inline void set_state(volatile char _state) {
  state = _state;
  pthread_cond_broadcast(&cond);
}
int cnt = 0;
void* producer_routine(void* arg) {
  // Wait for consumer to start
  wait_for_consumer_to_start();

  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  auto* value = (Value*)arg;
  int x;
 
  while (std::cin >> x) {
    pthread_mutex_lock(&mutex);
    cnt++;
    value -> update(x);
    set_state(PRODUCED);
    while (state != CONSUMED) {
      pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
  }

  pthread_mutex_lock(&mutex);
  set_state(FINISHED);
  pthread_mutex_unlock(&mutex);

  return NULL;
 
}
 
void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);	
  // notify about start
  set_state(STARTED);
  // allocate value for result
  Value* value = (Value*)arg;
  int* sum = new int;
  *sum = 0;

  // for every update issued by producer, read the value and add to sum
  while (state != FINISHED) {
	pthread_mutex_lock(&mutex);
    while (state == STARTED || state == CONSUMED) {
      pthread_cond_wait(&cond, &mutex);
	}                   
    if (state == PRODUCED) {
      *sum += value -> get();
      set_state(CONSUMED);
    }
    pthread_mutex_unlock(&mutex);
  }
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);	

  // return pointer to result
  return sum;
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  wait_for_consumer_to_start(); 

  pthread_t* consumer_thread = (pthread_t*)arg;
  while (state != FINISHED) {
    pthread_cancel(*consumer_thread);
  }
  return NULL;
}
 
int run_threads() {
  // start 3 threads and wait until they're done
  pthread_t producer_thread;
  pthread_t consumer_thread;
  pthread_t consumer_interruptor_thread;	
  Value value = Value();
  int* sum;

  pthread_create(&producer_thread, NULL, producer_routine, &value); 
  pthread_create(&consumer_thread, NULL, consumer_routine, &value); 
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread); 

  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, (void**)(&sum));
  pthread_join(consumer_interruptor_thread, NULL);
 
  // return sum of update values seen by consumer
  return *sum;
}
//#include <ctime> 
int main() {
//  freopen("stdin", "r", stdin);
//  double x = clock();

    std::cout << run_threads() << std::endl;
//  cout << clock() - x;
    return 0;
}
