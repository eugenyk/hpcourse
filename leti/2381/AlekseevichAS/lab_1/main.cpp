#include <pthread.h>
#include <iostream>
#include <unistd.h>
#include <sstream>

pthread_mutex_t mutex;
pthread_mutex_t mutex_i;

pthread_cond_t cond_consumer_ready;
pthread_cond_t cond_producer_ready;
pthread_cond_t cond_consumer_start;

pthread_t consumer;

bool producer_status = false;
bool consumer_status = false;

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
  producer_status = true;

  pthread_mutex_lock(&mutex);
  while(!consumer_status){
    pthread_cond_wait(&cond_consumer_start, &mutex);
  }
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
  Value *value = (Value*)arg;

  std::cout << "Enter <int> numbers: " << std::endl; 
  std::string input_numbers;
  std::getline(std::cin, input_numbers);
  std::stringstream stream(input_numbers);
  int n;
  
  while(stream >> n){
    value->update(n);
    pthread_cond_signal(&cond_producer_ready);
    pthread_cond_wait(&cond_consumer_ready, &mutex);
  }
  
  producer_status = false;
  pthread_cond_signal(&cond_producer_ready);
  pthread_mutex_unlock(&mutex);
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  
  pthread_mutex_lock(&mutex);
  pthread_mutex_lock(&mutex_i);
  consumer_status = true;
  Value *value = (Value*)arg;
  int *sum = new int;
  pthread_cond_broadcast(&cond_consumer_start);
  pthread_mutex_unlock(&mutex_i);
  
  pthread_cond_wait(&cond_producer_ready, &mutex);
  while(producer_status){
    *sum += value->get();
    pthread_cond_signal(&cond_consumer_ready);
    pthread_cond_wait(&cond_producer_ready, &mutex);
  }
  
  consumer_status = false;
  return sum;
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start
  pthread_mutex_lock(&mutex_i);
  while(!consumer_status){
    pthread_cond_wait(&cond_consumer_start, &mutex_i);
  }

  // interrupt consumer while producer is running                                          
  while (consumer_status){
      pthread_cancel(consumer);
  }
}
 
int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
  Value *value = new Value();
  int *sum;

  pthread_t producer;
  pthread_t interrupter;

  pthread_create(&producer, NULL, &producer_routine, value);
  pthread_create(&consumer, NULL, consumer_routine, value);
  pthread_create(&interrupter, NULL, &consumer_interruptor_routine, NULL);

  pthread_join(producer, NULL);
  pthread_join(consumer, (void**)&sum);
  pthread_join(interrupter, NULL);

  return *sum;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}