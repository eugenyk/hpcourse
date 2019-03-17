#include <iostream>
#include <pthread.h>
#include <vector>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <zconf.h>
#include "barrier.h"

pthread_barrier_t barrier;
__thread int consumer_sum;

class data_t {
public:
    pthread_cond_t read_condition = PTHREAD_COND_INITIALIZER;
    pthread_cond_t write_condition = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    std::vector<pthread_t> &consumersThread;

    data_t(unsigned int maxRand, std::vector<pthread_t> &threads) : maxRand(maxRand), consumersThread(threads) {
    }

    int get_value() {
        last_action = SAVE_VALUE;
        return value;
    }

    void set_value(int new_value) {
        last_action = UPDATED_VALUE;
        value = new_value;
    }

    void finish() {
        status = FINISH;
    }

    bool is_finish() {
        return status == FINISH;
    }

    void wait_reading_access() {
        while (last_action != UPDATED_VALUE) {
            pthread_cond_wait(&write_condition, &mutex);
        }
    }

    void wait_writing_access() {
        while (last_action != SAVE_VALUE) {
            pthread_cond_wait(&read_condition, &mutex);
        }
    }

    unsigned int maxRand = 0;
private:

    enum action_type {
        UPDATED_VALUE,
        SAVE_VALUE
    };
    enum status_type {
        INIT,
        FINISH
    };
    int value = 0;
    action_type last_action = SAVE_VALUE;
    status_type status = INIT;


};

void *producer_routine(void *arg) {
    // Wait for consumer to start
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    pthread_barrier_wait(&barrier);
    auto *data = (data_t *) arg;
    int cur_value = 0;
    while (std::cin >> cur_value) {
        pthread_mutex_lock(&data->mutex);
        data->wait_writing_access();
        data->set_value(cur_value);
        pthread_cond_signal(&data->write_condition);
        pthread_mutex_unlock(&data->mutex);
    }
    data->finish();
    pthread_cond_broadcast(&data->write_condition);
    pthread_mutex_unlock(&data->mutex);
    return nullptr;
}

void *consumer_routine(void *arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&barrier);
    consumer_sum = 0;
    auto *data = (data_t *) arg;
    while (!data->is_finish()) {
        pthread_mutex_lock(&data->mutex);
        data->wait_reading_access();
        consumer_sum += data->get_value();
        pthread_cond_signal(&data->read_condition);
        pthread_mutex_unlock(&data->mutex);
        auto sec = rand() % (data->maxRand + 1) * 1000;
        if (sec != 0) {
            usleep(sec);
        }
    }
    return &consumer_sum;
}

void *consumer_interruptor_routine(void *arg) {
    // wait for consumers to start
    // interrupt random consumer while producer is running
    pthread_barrier_wait(&barrier);
    auto *data = (data_t *) arg;
    while (!data->is_finish()) {
        unsigned long thread_num = (rand() % (data->consumersThread.size()));
        pthread_cancel(data->consumersThread[thread_num]);
    }
    return nullptr;
}

int run_threads(unsigned int n, unsigned int maxRand) {
    // start N threads and wait until they're done
    // return aggregated sum of values
    pthread_barrier_init(&barrier, nullptr, n + 2);
    std::vector<pthread_t> consumers(n);
    pthread_t producer;
    pthread_t interruptor;
    data_t data(maxRand, consumers);

    pthread_create(&producer, nullptr, producer_routine, &data);
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &data);
    for (int i = 0; i < n; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, &data);
    }
    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    int sum = 0;
    int *consumer_response;
    for (int i = 0; i < n; i++) {
        pthread_join(consumers[i], (void **) &consumer_response);
        sum += *consumer_response;
    }

    return sum;
}

int main(int argc, char *argv[]) {
    std::cout << run_threads(static_cast<unsigned int>(std::stoi(argv[1])),
                             static_cast<unsigned int>(std::stoi(argv[2]))) << std::endl;
}