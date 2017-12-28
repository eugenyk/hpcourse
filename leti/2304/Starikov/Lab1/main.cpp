#include <pthread.h>
#include <iostream>
#include <sstream>

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

/// global mutex for wait condition 'cond'
pthread_mutex_t mutex;

/// global wait condition
/// used for:
///  - consumer started event
///  - producer update value event
///  - consumer store value event
pthread_cond_t cond;

/// flag for waitCondition cond, used when the consumer started
volatile bool isConsumerStarted = false;

/// flag for waitCondition cond, used for value update and calculate, between producer/consumer
volatile bool currentValueApplied = false;

/// this flag will be set after producer ended his job
volatile bool producerEnded = false;

void waitConditionWithCheck(pthread_mutex_t &mutex, pthread_cond_t &cond, volatile bool &check)
{
  pthread_mutex_lock(&mutex);
  while (!check)
  {
      pthread_cond_wait(&cond, &mutex);
  }
  pthread_mutex_unlock(&mutex);
}

void* producer_routine(void* arg) {
  // Wait for consumer to start
  waitConditionWithCheck(mutex, cond, isConsumerStarted);

  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  Value *value = reinterpret_cast<Value*>(arg);

  std::string inputLine;
  getline(std::cin, inputLine);
  std::stringstream stringStream(inputLine);

  int readValue = 0;
  while (stringStream >> readValue)
  {
    value->update(readValue);
    currentValueApplied = false;
    pthread_cond_broadcast(&cond);
    waitConditionWithCheck(mutex, cond, currentValueApplied);
  }

  producerEnded = true;
  pthread_cond_broadcast(&cond);

}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  Value *value = reinterpret_cast<Value*>(arg);

  // notify about start
  isConsumerStarted = true;
  pthread_cond_broadcast(&cond);

  // allocate value for result
  int* result = new int(0);

  // for every update issued by producer, read the value and add to sum
  while(!producerEnded)
  {
    pthread_mutex_lock(&mutex);
    while (!producerEnded && currentValueApplied)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    if(producerEnded) break;

    *result += value->get();
    currentValueApplied = true;

    pthread_cond_broadcast(&cond);
  }

  // return pointer to result
  pthread_exit(reinterpret_cast<void*>(result));
}

void* consumer_interruptor_routine(void* arg) {
  pthread_t* consumerThreadPtr = reinterpret_cast<pthread_t*>(arg);

  // wait for consumer to start
  waitConditionWithCheck(mutex, cond, isConsumerStarted);

  // interrupt consumer while producer is running
  while(!producerEnded)
  {
      pthread_cancel(*consumerThreadPtr);
  }

  pthread_exit(NULL);
}

int run_threads() {
  pthread_t producerThread;
  pthread_t consumerThread;
  pthread_t interruptorThread;
  Value value;

  int *returnValuePointer;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&cond, NULL);

  // start 3 threads and wait until they're done
  pthread_create(&producerThread   , NULL, &producer_routine            , reinterpret_cast<void*>(&value));
  pthread_create(&consumerThread   , NULL, &consumer_routine            , reinterpret_cast<void*>(&value));
  pthread_create(&interruptorThread, NULL, &consumer_interruptor_routine, reinterpret_cast<void*>(&consumerThread));

  pthread_join(producerThread, NULL);
  pthread_join(consumerThread, reinterpret_cast<void**>(&returnValuePointer));
  pthread_join(interruptorThread, NULL);

  // return sum of update values seen by consumer
  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&cond);

  int returnValue = *returnValuePointer;
  delete returnValuePointer;

  return returnValue;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
