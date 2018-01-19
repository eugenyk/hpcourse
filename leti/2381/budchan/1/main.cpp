#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <unistd.h>

pthread_t producer, consumer, interrupter;
pthread_mutex_t mutex;
pthread_cond_t condp, condc;

bool is_consumer_started = false;
bool is_producer_finished = false;

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

    // Wait for consumer to start
    while (!is_consumer_started) {
        sleep(1);
    }

    Value *value = (Value*)arg;
    std::string arg_line = "";
    
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::cout << "Enter the values (e.g. 42 10 15): " << std::endl;
    std::getline(std::cin, arg_line);
    std::istringstream sline(arg_line);
    std::istream_iterator<std::string> begin(sline), end;

    std::vector<std::string> values_vector(begin, end);
    int values_to_handle = values_vector.size();
    int vector_size = values_to_handle;
    
    while (values_to_handle > 0) {			

        if ((vector_size - values_to_handle) > 0) 
            pthread_cond_wait(&condc, &mutex);
        else 
            pthread_mutex_lock(&mutex);		// first iteration

        int v = stoi(values_vector.at(vector_size - values_to_handle--));
        value->update(v);
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&condp);
    }
    pthread_cond_signal(&condp);
    is_producer_finished = true;
}
 
void* consumer_routine(void* arg) {
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    is_consumer_started = true;
    
    Value *value = (Value*)arg;
    int *sum = new int(0);

    while(!is_producer_finished) { 
        if (pthread_cond_wait(&condp, &mutex) == 0) {
            *sum += value->get();
            
            pthread_mutex_unlock(&mutex); 
            pthread_cond_signal(&condc);
        }	
    }
    return sum;
}
 
void* consumer_interruptor_routine(void* arg) {
    // interrupt consumer while producer is running
    while (!is_consumer_started) {
        sleep(1);
    }   
    while (is_consumer_started && !is_producer_finished) {
        pthread_cancel(consumer);
    }             
}
 
int run_threads() {
    // start 3 threads and wait until they're done
    Value *value = new Value();
    int *sum;
  
    pthread_create(&producer, NULL, &producer_routine, value);
    pthread_create(&consumer, NULL, &consumer_routine, value);
    pthread_create(&interrupter, NULL, &consumer_interruptor_routine, NULL);

    // return sum of update values seen by consumer
    pthread_join(producer, NULL);
    pthread_join(consumer, (void**)&sum); 
    pthread_join(interrupter, NULL);

    return *(int*) sum;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
