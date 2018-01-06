#include <pthread.h>
#include <iostream>
using namespace std;
#define DBG(x) cout << #x << " = " << (x) << endl;


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

pthread_t producer, consumer, interruptor;
bool is_consumer_started = false;
// This bool is used without synchronization. 
// I *hope* that `volatile` keyword protects from visibility problems.
// Related links:
//   http://en.cppreference.com/w/cpp/language/memory_model
//   https://stackoverflow.com/a/6996259/5338270
volatile bool is_input_exhausted = false;
bool is_consumer_answered = false;
int input_sequence_number = 0;

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t consumer_started = PTHREAD_COND_INITIALIZER;
pthread_cond_t number_received = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_got_number = PTHREAD_COND_INITIALIZER;


void wait_consumer_start()
{
  pthread_mutex_lock(&value_mutex);
  while (not is_consumer_started)
    pthread_cond_wait(&consumer_started, &value_mutex);
  pthread_mutex_unlock(&value_mutex);
}

 
void* producer_routine(void* arg) {
  auto value_holder = static_cast<Value*>(arg);
  int current_value;

  // Wait for consumer to start
  wait_consumer_start();

  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  while (cin >> current_value)
  {
    pthread_mutex_lock(&value_mutex);

    is_consumer_answered = false;
    value_holder->update(current_value);
    ++input_sequence_number;
    pthread_cond_signal(&number_received);

    while (not is_consumer_answered)
      pthread_cond_wait(&consumer_got_number, &value_mutex);

    pthread_mutex_unlock(&value_mutex);
  }

  pthread_mutex_lock(&value_mutex);
  is_input_exhausted = true;
  pthread_cond_signal(&number_received);
  pthread_mutex_unlock(&value_mutex);

  return NULL;
}
 
void* consumer_routine(void* arg) {
  auto value_holder = static_cast<Value*>(arg);
  // allocate value for result
  int* result = new int(0);
  int inputs_received = 0;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

  // notify about start
  pthread_mutex_lock(&value_mutex);
  is_consumer_started = true;
  pthread_cond_broadcast(&consumer_started);
  pthread_mutex_unlock(&value_mutex);

  // for every update issued by producer, read the value and add to sum
  while (true) 
  {
    pthread_mutex_lock(&value_mutex);
    while (inputs_received == input_sequence_number and not is_input_exhausted)
      pthread_cond_wait(&number_received, &value_mutex);

    if (is_input_exhausted)
    {
      pthread_mutex_unlock(&value_mutex);
      break;
    }

    ++inputs_received;
    *result += value_holder->get();

    is_consumer_answered = true;
    pthread_cond_signal(&consumer_got_number);

    pthread_mutex_unlock(&value_mutex);
  }

  // return pointer to result
  return result;
}
 
void* consumer_interruptor_routine(void* arg) {
  (void)arg;
  // wait for consumer to start
  wait_consumer_start();

  while (not is_input_exhausted) 
  {
    // interrupt consumer while producer is running   
    pthread_cancel(consumer);
  }
  
  return NULL;
}
 
int run_threads() {
  auto value = new Value();
  int* consumer_result_ptr;

  // start 3 threads and wait until they're done
  pthread_create(&producer, NULL, producer_routine, value);
  pthread_create(&consumer, NULL, consumer_routine, value);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);

  pthread_join(producer, NULL);
  pthread_join(consumer, (void**)&consumer_result_ptr);
  pthread_join(interruptor, NULL);
 
  // return sum of update values seen by consumer
  auto consumer_result = *consumer_result_ptr;

  delete consumer_result_ptr;
  delete value;

  return consumer_result;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
