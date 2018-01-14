#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <errno.h>

class Value 
{
public:
    Value() : _value(0) { }
 
    void update(int value) 
    {
        _value = value;
    }
 
    int get() const 
    {
        return _value;
    }
 
private:
    int _value;
};

pthread_mutex_t mutex;
pthread_cond_t cond;

bool isConsumerStarted = false;

bool isReadyToCompute = false;
bool isRun = false;
bool isReadyToClose= false;


 
void* producer_routine(void* arg) 
{
  // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!isConsumerStarted)
        pthread_cond_wait(&cond, &mutex);

    pthread_mutex_unlock(&mutex);

    Value *value = reinterpret_cast<Value *>(arg);
    std::vector<int> data;

  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string line;
    getline(std::cin, line);
    std::istringstream is(line);
    int console_data;

    //transform string to int list
    while (is >> console_data) {
        data.emplace_back(console_data);
    }

    unsigned counter = 0;
    while(isRun && !isReadyToClose){
        if(!isReadyToCompute){
            value->update(data[counter]);
            counter++;
            if(counter >= data.size())
                isReadyToClose = true;
            isReadyToCompute = true;
        }
    }

    pthread_exit(NULL);
}
 
void* consumer_routine(void* arg) 
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL); //запрет на завершение потока

    Value *value = reinterpret_cast<Value *>(arg);
    isRun = true;
  // notify about start
    isConsumerStarted = true;
    pthread_cond_broadcast(&cond);
  // allocate value for result
    int res = 0;
  // for every update issued by producer, read the value and add to sum
      while(isRun){
        if(isReadyToCompute){
            int cur_data = value->get();
            res += cur_data;
            //cout << "val = " << x << "  res = " << result << endl;

            if(isReadyToClose)
                isRun = false;

            isReadyToCompute = false;
        }

    }
  // return pointer to result
    pthread_exit((void*)res);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL); //разрешение на завершение потока
}
 
void* consumer_interruptor_routine(void* arg) 
{
    auto consumerPointer = (pthread_t*)arg;
  // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while(!isConsumerStarted)
        pthread_cond_wait(&cond, &mutex);
    pthread_mutex_unlock(&mutex);
    //cout << "consumer_interruptor_routine start\n";
    while(isRun){
        // interrupt consumer while producer is running
        pthread_cancel(*consumerPointer);
    }
    pthread_exit(NULL);                                        
}
 
int run_threads() 
{
    pthread_t thread_producer;
    pthread_t thread_consumer;
    pthread_t thread_interruptor;

    pthread_attr_t attr;

    Value arg;
    int sum;

    int result_thread;

    // start 3 threads and wait until they're done
    pthread_attr_init(&attr);

    result_thread = pthread_create(&thread_producer, &attr, &producer_routine, &arg);
    if (result_thread != 0) {
        perror("Creating the first thread");
        return EXIT_FAILURE;
    }

    result_thread = pthread_create(&thread_consumer, &attr, &consumer_routine, &arg);
    if (result_thread != 0) {
        perror("Creating the second thread");
        return EXIT_FAILURE;
    }

    result_thread = pthread_create(&thread_interruptor, &attr, &consumer_interruptor_routine, &thread_consumer);
    if (result_thread != 0) {
        perror("Creating the third thread");
        return EXIT_FAILURE;
    }

    result_thread = pthread_join(thread_producer, NULL);
    if (result_thread != 0) {
        perror("Joining the first thread");
        return EXIT_FAILURE;
    }

    result_thread = pthread_join(thread_consumer, (void **)&sum);
    if (result_thread != 0) {
        perror("Joining the second thread");
        return EXIT_FAILURE;
    }

    result_thread = pthread_join(thread_interruptor, NULL);
    if (result_thread != 0) {
        perror("Joining the third thread");
        return EXIT_FAILURE;
    }
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&cond);

    // return sum of update values seen by consumer
    return sum;
}
 
int main() 
{
    std::cout << "Enter the sequence for addition: " << std::endl;
    std::cout << run_threads() << std::endl;
    return 0;
}