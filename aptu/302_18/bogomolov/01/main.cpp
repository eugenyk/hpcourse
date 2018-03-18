#include <pthread.h>
#include <iostream>

#define NOT_STARTED 0
#define CONSUMED_EVERYTHING 1
#define WAITING_FOR_CONSUMER 2
#define FINISHED 3

class Value {
public:

    Value() : _value(0) {}

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};

int status;
pthread_cond_t cond;
pthread_mutex_t mutex;

void* producer_routine(void* arg) {
    // Unpack arguments
    auto* value = (Value*)arg;

    // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while(status == NOT_STARTED) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    int number;
    while(std::cin >> number) {
        pthread_mutex_lock(&mutex);
        value->update(number);
        status = WAITING_FOR_CONSUMER;
        pthread_cond_broadcast(&cond);
        while(status != CONSUMED_EVERYTHING) {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);
    status = FINISHED;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
    return nullptr;
}

void* consumer_routine(void* arg) {
    // Unpack arguments
    auto* value = (Value*)arg;

    // notify about start
    pthread_mutex_lock(&mutex);
    status = CONSUMED_EVERYTHING;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    // for every update issued by producer, read the value and add to sum
    auto* result = new int;
    *result = 0;
    int local_status = status;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    while(local_status != FINISHED) {
        pthread_mutex_lock(&mutex);
        while(status == CONSUMED_EVERYTHING) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (status == WAITING_FOR_CONSUMER) {
            *result += value->get();
            status = CONSUMED_EVERYTHING;
            pthread_cond_broadcast(&cond);
        }
        local_status = status;
        pthread_mutex_unlock(&mutex);
    }
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

    return result;
}

void* consumer_interruptor_routine(void* arg) {
    auto consumer = *(pthread_t*)arg;

    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while(status == NOT_STARTED) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    // interrupt consumer while producer is running
    while(status != FINISHED) {
        pthread_cancel(consumer);
    }
    return nullptr;
}

int run_threads() {
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer
    Value value;
    int* result;
    pthread_t consumer, producer, consumer_interruptor;
    pthread_create(&producer, nullptr, &producer_routine, &value);
    pthread_create(&consumer, nullptr, &consumer_routine, &value);
    pthread_create(&consumer_interruptor, nullptr, &consumer_interruptor_routine, &consumer);
    pthread_join(producer, nullptr);
    pthread_join(consumer, (void**)(&result));
    pthread_join(consumer_interruptor, nullptr);
    return *result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}