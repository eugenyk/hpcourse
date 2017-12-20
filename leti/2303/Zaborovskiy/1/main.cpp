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

volatile bool consumerRunning = false;
pthread_cond_t consumerWaiting;
pthread_mutex_t mutex;

pthread_cond_t producerPosted;
volatile bool producerPostedNewVal = false;
volatile bool lastVal = false;
volatile bool consumerRead = false;


typedef struct arg_wrapper_tag {
    int argc;
    char** argv;
    Value* value;
} arg_wrapper_t;

void* producer_routine(void* arg) {
    arg_wrapper_t* args = (arg_wrapper_t*) arg;
  // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!consumerRunning) {
        pthread_cond_wait(&consumerWaiting, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    cout << "PRODUCER: consumer started" << endl;
    Value* value = args->value;
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    for (int i = 0; i < args->argc; i++) {
        char* arg = args->argv[i];//just assume its a number, ok?
        int val = std::stoi(arg);
        pthread_mutex_lock(&mutex);
        // tell we posted new val and notify
        value->update(val);
        producerPostedNewVal = true;
        if (i == args->argc - 1) {
            // there won't be any more of us
            lastVal = true;
        }
        pthread_cond_signal(&producerPosted);

        //let the consumer do it's job
        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while (!consumerRead) {
            pthread_cond_wait(&consumerWaiting, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        cout << "PRODUCER: consumer handled value" << endl;
        consumerRead = false;
    }
    pthread_exit(NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    arg_wrapper_t* args = (arg_wrapper_t*) arg;

  // notify about start
    consumerRunning = true;
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&consumerWaiting);
    pthread_mutex_unlock(&mutex);
    cout << "CONSUMER: consumer started" << endl;
    int sum = 0;
    const Value* value = args->value;

    while (!lastVal) {
        pthread_mutex_lock(&mutex);
        while (!producerPostedNewVal) {
            pthread_cond_wait(&producerPosted, &mutex);
        }
        producerPostedNewVal = false;
        cout << "CONSUMER: producer posted new val" << endl;
        // for every update issued by producer, read the value and add to sum
        int val = value->get();
        sum = sum + val;
        consumerRead = true;
        cout << "CONSUMER: consumer handled value" << endl;
        pthread_cond_signal(&consumerWaiting);
        pthread_mutex_unlock(&mutex);
    }
    // return pointer to result
    pthread_exit((void*) sum);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

void* consumer_interruptor_routine(void* arg) {
  auto consumerThread = (pthread_t*)arg;
  // wait for consumer to start
  pthread_mutex_lock(&mutex);
  while (!consumerRunning) {
      pthread_cond_wait(&consumerWaiting, &mutex);
  }
  pthread_mutex_unlock(&mutex);

  cout << "INTERRUPTOR: consumer started" << endl;
  while (!lastVal) {
      cout << "INTERRUPTOR: try interript consumer" << endl;
    pthread_cancel(*consumerThread);
  }
  pthread_exit(NULL);
  // interrupt consumer while producer is running
}


int run_threads(int argc, char** argv) {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&consumerWaiting, NULL);
    pthread_cond_init(&producerPosted, NULL);

    pthread_t producerThread;
    pthread_t consumerThread;
    pthread_t interruptorThread;
  // start 3 threads and wait until they're done

    Value value;

    arg_wrapper_t producerArgs = {
        argc - 1, &argv[1], &value
    };
    pthread_create(&producerThread, NULL, producer_routine, &producerArgs);
    pthread_create(&consumerThread, NULL, consumer_routine, &producerArgs);
    pthread_create(&interruptorThread, NULL, consumer_interruptor_routine, &consumerThread);

  // return sum of update values seen by consumer
    pthread_join(producerThread, NULL);
    int sum;
    pthread_join(consumerThread, (void **)&sum);
    pthread_join(interruptorThread, NULL);
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&consumerWaiting);
    pthread_cond_destroy(&producerPosted);
    return sum;
}
int main(int argc, char** argv) {

    std::cout << "Result is " << run_threads(argc, argv) << std::endl;

    return 0;
}
