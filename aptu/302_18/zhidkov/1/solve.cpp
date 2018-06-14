#include <pthread.h>
#include <iostream>
#include <vector>
using namespace std;

#define NUMBERS_END -1

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

pthread_mutex_t mmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool hasConsumerStarted = false;
bool hasConsumerFinished = false;
int producerCnt = 0;
int consumerCnt = 0;

 
void* producer_routine(void* arg) {
  // Wait for consumer to start
 
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  Value *value = (Value *) arg;

  // wait for consumer
  pthread_mutex_lock(&mmutex);
  while (!hasConsumerStarted) pthread_cond_wait(&cond, &mmutex);
  pthread_mutex_unlock(&mmutex);

  // read data
  int n;
  cin >> n;
  std::vector<int> numbers(n);
  for (int i = 0; i < n; i++) {
    cin >> numbers[i];
  }
  numbers.push_back(NUMBERS_END);

  // send data
  for (size_t i = 0; i < numbers.size(); i++) {
    // send int
    pthread_mutex_lock(&mmutex);
    value->update(numbers[i]);
    producerCnt++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mmutex);

    // wait for consumer
    pthread_mutex_lock(&mmutex);
    while (producerCnt != consumerCnt) pthread_cond_wait(&cond, &mmutex);
    pthread_mutex_unlock(&mmutex);

  }

}
 
void* consumer_routine(void* arg) {
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result

  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  Value *value = (Value *) arg;

  pthread_mutex_lock(&mmutex);
  hasConsumerStarted = true;
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&mmutex);

  int *sum = new int[1];
  bool exit = false;
  while (1) {
    pthread_mutex_lock(&mmutex);
    while (producerCnt == consumerCnt) pthread_cond_wait(&cond, &mmutex);
    int val = value->get();
    if (val == NUMBERS_END) {
      exit = true;
    } else {
      sum[0] += val;
    }
    consumerCnt++;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mmutex);

    if (exit) {
      break;
    }

  }
  hasConsumerFinished = true;
  pthread_exit(sum);
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  pthread_mutex_lock(&mmutex);
  while (!hasConsumerStarted) pthread_cond_wait(&cond, &mmutex);
  pthread_mutex_unlock(&mmutex);

  pthread_t *consumer = (pthread_t *)arg;

  while (!hasConsumerFinished) {
    pthread_cancel(*consumer);
  }

  
 
  // interrupt consumer while producer is running                                          
}
 
int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer

  Value value;
  pthread_t producer, consumer, interruptor;
  pthread_create(&producer, NULL, producer_routine, (void *) &value);
  pthread_create(&consumer, NULL, consumer_routine, (void *) &value);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, (void *) &consumer);

  pthread_join(producer, NULL);
  void *conumerSt;
  pthread_join(consumer, &conumerSt);
  int *sum = (int *) conumerSt;
  int su = sum[0];
  free(sum);
  pthread_join(interruptor, NULL);
  return su;
}
 
int main() {

    std::cout << run_threads() << std::endl;
    return 0;
}