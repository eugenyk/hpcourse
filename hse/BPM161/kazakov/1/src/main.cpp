#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <array>
#include <vector>
#include <sstream>
#include <time.h>
#include "../include/consumer_parameters.h"
#include "../include/random.h"
#include "../include/value_storage.h"
#include "../include/no_value_exception.h"


int started_consumers_number = 0;
pthread_mutex_t started_consumers_number_access = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumer_started = PTHREAD_COND_INITIALIZER;
pthread_cond_t all_consumers_started = PTHREAD_COND_INITIALIZER;

volatile bool is_producer_finished = false;
thread_local int consumer_sum = 0;

void *producer_routine(void *arg) {
    auto *value = (value_storage *) arg;
    pthread_mutex_lock(&started_consumers_number_access);
    while (started_consumers_number==0) {
        pthread_cond_wait(&consumer_started, &started_consumers_number_access);
    }
    pthread_mutex_unlock(&started_consumers_number_access);

    std::string values;
    getline(std::cin, values);
    std::istringstream values_stream(values);
    int val;
    while (values_stream >> val) {
        value->update(val);
    }
    is_producer_finished = true;
    value->close();

    return nullptr;
}

void msleep(int ms) {
    struct timespec ts_sleep =
        {
            ms/1000,
            (ms%1000)*1000000L
        };
    nanosleep(&ts_sleep, nullptr);
}

void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    auto *parameters = (consumer_parameters *) arg;
    class random random(0, parameters->max_sleep_time);

    pthread_mutex_lock(&started_consumers_number_access);
    started_consumers_number++;
    pthread_cond_signal(&consumer_started);
    if (parameters->consumers_number==started_consumers_number) {
        pthread_cond_signal(&all_consumers_started);
    }
    pthread_mutex_unlock(&started_consumers_number_access);

    try {
        while (!is_producer_finished || parameters->value.has_value()) {
            consumer_sum += parameters->value.get();
            msleep(random.next_int());
        }
    }
    catch (no_value_exception& exception) {}

    return (void *) &consumer_sum;
}

void *consumer_interruptor_routine(void *arg) {
    auto *consumers = static_cast<std::vector<pthread_t> *>(arg);
    int consumers_number = static_cast<int>(consumers->size());
    class random random(0, consumers_number - 1);

    pthread_mutex_lock(&started_consumers_number_access);
    while (consumers_number!=started_consumers_number) {
        pthread_cond_wait(&all_consumers_started, &started_consumers_number_access);
    }
    pthread_mutex_unlock(&started_consumers_number_access);

    while (!is_producer_finished) {
        auto consumer_to_interrupt = static_cast<unsigned long>(random.next_int());
        pthread_cancel((*consumers)[consumer_to_interrupt]);
    }

    return nullptr;
}

int run_threads(long threads_number, int max_sleep_time) {
    std::vector<pthread_t> consumers(static_cast<unsigned long>(threads_number));
    value_storage value;
    consumer_parameters parameters(value, max_sleep_time, threads_number);
    for (pthread_t &consumer : consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, (void *) &parameters);
    }

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, (void *) &value);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, (void *) &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    int sum = 0;
    for (pthread_t &consumer : consumers) {
        int *consumer_sum;
        pthread_join(consumer, (void **) &consumer_sum);
        sum += *consumer_sum;
    }

    return sum;
}

void print_usage() {
    std::cout << "Usage: ./lab1 <consumer threads number> <consumer max sleep time>" << std::endl;
}

int main(int argc, char *argv[]) {
    if (argc!=3) {
        print_usage();
        return 1;
    }

    long threads_number = strtol(argv[1], nullptr, 10);
    if (threads_number <= 0) {
        std::cout << "Consumer threads number should be positive\n";
        print_usage();
        return 1;
    }

    long max_sleep_time = strtol(argv[2], nullptr, 10);
    if (max_sleep_time < 0) {
        std::cout << "Consumer max sleep time should be non-negative\n";
        print_usage();
        return 1;
    }

    std::cout << run_threads(threads_number, static_cast<int>(max_sleep_time)) << std::endl;
    return 0;
}