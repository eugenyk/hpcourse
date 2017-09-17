#include <QCoreApplication>
#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include "value.h"
#include <unistd.h>
#include <semaphore.h>

using namespace std;

Value val = Value();
int int_msg[] ={0,1,2,3,4,5,6,7,8,9};

pthread_t producerThread;
pthread_t consumerThread;
pthread_t interruptorThread;

pthread_mutex_t myMutex;
sem_t producerSemaphore;
sem_t interruptorSemaphore;

bool isReadyToCompute = false;
bool isRun = false;

void* producer_routine(void* arg) {
    cout << "producer_routine start\n";
  // Wait for consumer to start
    sem_wait(&producerSemaphore);
    cout << "consumer is started\n";
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    auto dataForUpdate =  (int*) arg;
    int counter = 0;
    while(isRun){
        if(!isReadyToCompute){
            pthread_mutex_lock(&myMutex);
            isReadyToCompute = true;
            if(counter == 10)
                isRun=false;
            val.update(dataForUpdate[counter]);
            counter++;
            pthread_mutex_unlock(&myMutex);
        }
    }
    pthread_exit(NULL);
}

void* consumer_routine(void* arg) {
  // notify about start
    cout << "consumer_routine start\n";

    int s;
    s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (s != 0)
        cout <<  "pthread_setcancelstate ERROR";
    isRun = true;
    sem_post(&producerSemaphore);
    sem_post(&interruptorSemaphore);
  // allocate value for result
    int result = 0;
  // for every update issued by producer, read the value and add to sum

    while(isRun){
        if(isReadyToCompute){
            sleep(2);
            int x = val.get();
            result += x;
            cout << "val = " << x << "  res = " << result << endl;
            isReadyToCompute = false;
        }
    }
    s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    if (s != 0)
        cout <<  "pthread_setcancelstate ERROR";
    pthread_exit((void*)result);

    // return pointer to result

}

void* consumer_interruptor_routine(void* arg) {
//  // wait for consumer to start
    sem_wait(&interruptorSemaphore);
    cout << "consumer_interruptor_routine start\n";
        while(isRun){
            // interrupt consumer while producer is running
            while(isRun && isReadyToCompute){
                pthread_cancel(consumerThread);
            }
        }

    pthread_exit(NULL);
}

int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
    sem_init(&producerSemaphore, 0, 0);
    sem_init(&interruptorSemaphore, 0, 0);
    int ret_join;
    pthread_mutex_init(&myMutex,0);
    pthread_create(&producerThread, NULL, &producer_routine, (void*) int_msg);
    pthread_create(&consumerThread, NULL, &consumer_routine, NULL);
    pthread_create(&interruptorThread, NULL, &consumer_interruptor_routine, NULL);
    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, (void **)&ret_join);
    pthread_join(interruptorThread, NULL);
    pthread_mutex_destroy(&myMutex);
    cout << "RESULT  " << ret_join << endl;
    sem_destroy(&producerSemaphore);
    sem_destroy(&interruptorSemaphore);
    return ret_join;
}

int main() {
    cout <<" RESULT = " << run_threads() << std::endl;
    return 0;
}
