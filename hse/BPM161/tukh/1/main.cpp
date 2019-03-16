#include <pthread.h>
 
void* producer_routine(void* arg) {
  // Wait for consumer to start
 
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
}
 
void* consumer_routine(void* arg) {
  // notify about start
  // for every update issued by producer, read the value and add to sum
  // return pointer to result (for particular consumer)
}
 
void* consumer_interruptor_routine(void* arg) {
  // wait for consumers to start
 
  // interrupt random consumer while producer is running                                          
}
 
int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values
 
  return 0;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}