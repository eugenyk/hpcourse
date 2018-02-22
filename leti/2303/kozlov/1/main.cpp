#include <iostream>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sstream>

using namespace std;

pthread_mutex_t mutex;
pthread_cond_t consumerIsWaiting;
pthread_cond_t producerHasProduced;

volatile bool consumerIsRunning;
volatile bool isLastValue;
volatile bool consumerHasRead;
volatile bool newValProduced;

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

Value* value = new Value();

typedef struct {
    int argc;
    char** argv;
} InputData;


void* producer_routine(void *arg) {
    InputData* input = (InputData*)arg;

    pthread_mutex_lock(&mutex);
    while (!consumerIsRunning) {
        pthread_cond_wait(&consumerIsWaiting, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    cout << "Producer: consumer started" << endl;

    for (int i = 0; i < input -> argc; i++) {
        int val = std::stoi(input->argv[i]);
        pthread_mutex_lock(&mutex);
        value->update(val);
        newValProduced = true;

        if (i == input -> argc - 1) {
            isLastValue = true;
        }
        pthread_cond_signal(&producerHasProduced);

        pthread_mutex_unlock(&mutex);

        pthread_mutex_lock(&mutex);
        while (!consumerHasRead) {
            pthread_cond_wait(&consumerIsWaiting, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        cout << "Producer: read new value" << endl;
        consumerHasRead = false;
    }

    pthread_exit(NULL);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    consumerIsRunning = true;
    pthread_mutex_lock(&mutex);
    pthread_cond_broadcast(&consumerIsWaiting);
    pthread_mutex_unlock(&mutex);
    cout << "Consume: i have started" << endl;

    int sum = 0;

    while (!isLastValue) {
        pthread_mutex_lock(&mutex);
        while (!newValProduced) {
            pthread_cond_wait(&producerHasProduced, &mutex);
        }
        newValProduced = false;
        cout << "Consumer: got new val" << endl;
        int val = value->get();
        sum += val;
        consumerHasRead = true;
        cout << "Consumer: i have added new valute to sum" << endl;

        pthread_cond_signal(&consumerIsWaiting);
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit((void*) sum);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
}

void* consumer_interruptor_routine(void* arg) {
    auto threadToInterrupt = (pthread_t*) arg;
    pthread_mutex_lock(&mutex);
    while (!consumerIsRunning) {
        pthread_cond_wait(&consumerIsWaiting, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    cout << "Interruptor: consumer is ready to be interrupted" << endl;

    while (!isLastValue) {
        pthread_cancel(*threadToInterrupt);
        cout << "Iterruptor: trying to shut down consumer" << endl;
    }
    pthread_exit(NULL);
}

int run_threads(int argc, char** argv) {
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer

    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&consumerIsWaiting, NULL);
    pthread_cond_init(&producerHasProduced, NULL);

    int sum;
    pthread_t consumerThread;
    pthread_t interruptorThread;
    pthread_t producerThread;

    InputData newInput = {argc - 1, &argv[1]};

    pthread_create(&consumerThread, NULL, consumer_routine, NULL);
    pthread_create(&producerThread, NULL, producer_routine, &newInput);
    pthread_create(&interruptorThread, NULL, consumer_interruptor_routine, &consumerThread);

    pthread_join(producerThread, NULL);
    pthread_join(consumerThread, (void **) &sum);
    pthread_join(interruptorThread, NULL);

    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&consumerIsWaiting);
    pthread_cond_destroy(&producerHasProduced);
    return sum;
}

int main(int argc, char** argv) {

    consumerIsRunning = false;
    isLastValue = false;
    consumerHasRead = false;
    newValProduced = false;

    std::cout << run_threads(argc, argv) << std::endl;
    return 0;
}



