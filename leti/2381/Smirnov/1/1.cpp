#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>
#include <vector>
using namespace std;

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


pthread_mutex_t myMutex;
pthread_cond_t cond;
volatile bool isConsumerStarted = false;

volatile bool isReadyToCompute = false;
volatile bool isRun = false;
volatile bool isReadyToClose= false;

void* producer_routine(void* arg) {

  // Wait for consumer to start
    pthread_mutex_lock(&myMutex);
    while (!isConsumerStarted)
        pthread_cond_wait(&cond, &myMutex);
    pthread_mutex_unlock(&myMutex);
    //cout << "producer_routine start\n";
    Value *value = reinterpret_cast<Value *>(arg);
    std::vector<int> inputValues;


     // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string line;
    getline(std::cin, line);
    std::istringstream is(line);
    int input;
    //transform string to int list
    while (is >> input) {
        inputValues.emplace_back(input);
    }

    unsigned counter = 0;
    while(isRun && !isReadyToClose){
        if(!isReadyToCompute){
            value->update(inputValues[counter]);
            counter++;
            if(counter >= inputValues.size())
                isReadyToClose = true;
            isReadyToCompute = true;
        }
    }
    pthread_exit(NULL);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    Value *value = reinterpret_cast<Value *>(arg);

    isRun = true;
    // notify about start
    isConsumerStarted = true;
    pthread_cond_broadcast(&cond);
    //cout << "consumer_routine start\n";

  // allocate value for result
    int result = 0;
  // for every update issued by producer, read the value and add to sum

    while(isRun){
        if(isReadyToCompute){
            int x = value->get();
            result += x;
            //cout << "val = " << x << "  res = " << result << endl;

            if(isReadyToClose)
                isRun = false;

            isReadyToCompute = false;
        }

    }
    // return pointer to result
    pthread_exit((void*)result);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

void* consumer_interruptor_routine(void* arg) {
    auto consumerPointer = (pthread_t*)arg;
  // wait for consumer to start
    pthread_mutex_lock(&myMutex);
    while(!isConsumerStarted)
        pthread_cond_wait(&cond, &myMutex);
    pthread_mutex_unlock(&myMutex);
    //cout << "consumer_interruptor_routine start\n";
    while(isRun){
        // interrupt consumer while producer is running
        pthread_cancel(*consumerPointer);
    }
    pthread_exit(NULL);
}

int run_threads() {
    pthread_t producerThread;
    pthread_t consumerThread;
    pthread_t interruptorThread;
    Value value;
  // start 3 threads and wait until they're done

    int ret_join;
    pthread_mutex_init(&myMutex,0);
    pthread_cond_init(&cond,NULL);
    pthread_create(&producerThread, NULL, &producer_routine, &value);
    pthread_create(&consumerThread, NULL, &consumer_routine, &value);
    pthread_create(&interruptorThread, NULL, &consumer_interruptor_routine, &consumerThread);
    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, (void **)&ret_join);
    pthread_join(interruptorThread, NULL);
    pthread_mutex_destroy(&myMutex);
    pthread_cond_destroy(&cond);
    // return sum of update values seen by consumer
    return ret_join;
}

int main() {
    cout << run_threads() << std::endl;
    return 0;
}
