#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <vector>
#include "pthread_barrier.h"

unsigned int N_consumer;
unsigned int milliseconds;

thread_local int part_sum;

int number_of_digits(int x) {
    int length = 1;
    while (x /= 10)
        length++;
    return length;
}

struct producer_arg_t {
    pthread_barrier_t *consumer_barrier;
    int *p_shared_int;
    int *p_shared_int_delivery_status;
    pthread_mutex_t *p_shared_int_mutex;
    pthread_cond_t *p_shared_int_cond_produce;
    pthread_cond_t *p_shared_int_cond_consume;
    std::vector<pthread_t*> consumers;
};

void *producer_routine(void *arg) {
    // Wait for consumer to start
    auto parg = (producer_arg_t *) arg;
    pthread_barrier_wait(parg->consumer_barrier);
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string input;
    getline(std::cin, input);
    for (size_t i = 0; i < input.length(); ++i) {
        pthread_mutex_lock(parg->p_shared_int_mutex);
        *parg->p_shared_int_delivery_status = 0;
        int cur_int = std::stoi(input.substr(i));
        i += number_of_digits(cur_int);
        *parg->p_shared_int = cur_int;
        pthread_cond_signal(parg->p_shared_int_cond_produce);
        while (*parg->p_shared_int_delivery_status != 1) {
            pthread_cond_wait(parg->p_shared_int_cond_consume, parg->p_shared_int_mutex);
        }
        pthread_mutex_unlock(parg->p_shared_int_mutex);
    }

    pthread_mutex_lock(parg->p_shared_int_mutex);
    *parg->p_shared_int_delivery_status = -1;
    pthread_cond_broadcast(parg->p_shared_int_cond_produce);
    pthread_mutex_unlock(parg->p_shared_int_mutex);

    return nullptr;
}

void *consumer_routine(void *arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    auto parg = (producer_arg_t *) arg;
    pthread_barrier_wait(parg->consumer_barrier);

    while (true) {
        pthread_mutex_lock(parg->p_shared_int_mutex);
        while (*parg->p_shared_int_delivery_status == 1) {
            pthread_cond_wait(parg->p_shared_int_cond_produce, parg->p_shared_int_mutex);
        }
        if (*parg->p_shared_int_delivery_status == -1) {
//            std::cout << "part_sum" << part_sum << std::endl;
            pthread_mutex_unlock(parg->p_shared_int_mutex);
            int *local_sum = new int;
            *local_sum = part_sum;
            return local_sum;
        }
        part_sum += *parg->p_shared_int;
        *parg->p_shared_int_delivery_status = 1;
        pthread_cond_signal(parg->p_shared_int_cond_consume);
        int sm = rand() % (milliseconds + 1);
//        std::cout << "slep:" << sm << std::endl;
        pthread_mutex_unlock(parg->p_shared_int_mutex);
        usleep(sm);
    }
}

void *consumer_interruptor_routine(void *arg) {
    // wait for consumers to start

    auto parg = (producer_arg_t *) arg;
    pthread_barrier_wait(parg->consumer_barrier);

    while (true) {
        pthread_mutex_lock(parg->p_shared_int_mutex);
        if (*parg->p_shared_int_delivery_status == -1) {
            pthread_mutex_unlock(parg->p_shared_int_mutex);
            return nullptr;
        }
        pthread_mutex_unlock(parg->p_shared_int_mutex);
        pthread_cancel(*parg->consumers[rand() % N_consumer]);
    }
    // interrupt random consumer while producer is running
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values
    int sum = 0;
    producer_arg_t arg;
    pthread_barrier_t consumer_barrier;
    pthread_barrier_init(&consumer_barrier, nullptr, N_consumer + 2);
    arg.consumer_barrier = &consumer_barrier;
    int *p_shared_int = new int;
    int *p_shared_int_delivery_status = new int;
    *p_shared_int_delivery_status = 1;
    arg.p_shared_int = p_shared_int;
    arg.p_shared_int_delivery_status = p_shared_int_delivery_status;
    pthread_mutex_t p_shared_int_mutex;
    pthread_mutex_init(&p_shared_int_mutex, nullptr);
    arg.p_shared_int_mutex = &p_shared_int_mutex;
    pthread_cond_t p_shared_int_cond_produce;
    pthread_cond_t p_shared_int_cond_consume;
    pthread_cond_init(&p_shared_int_cond_consume, nullptr);
    pthread_cond_init(&p_shared_int_cond_produce, nullptr);
    arg.p_shared_int_cond_produce = &p_shared_int_cond_produce;
    arg.p_shared_int_cond_consume = &p_shared_int_cond_consume;

    pthread_t producer;
    pthread_t consumer_intr;
    pthread_t consumer[N_consumer];
    pthread_create(&producer, nullptr, producer_routine, &arg);
    pthread_create(&consumer_intr, nullptr, consumer_interruptor_routine, &arg);
    for (int i = 0; i < N_consumer; i++) {
        arg.consumers.push_back(&consumer[i]);
        pthread_create(&consumer[i], nullptr, consumer_routine, &arg);
    }
    for (pthread_t pthread : consumer) {
        void *sn;
        pthread_join(pthread, &sn);
        sum += *((int *) sn);
    }
    pthread_cond_destroy(&p_shared_int_cond_consume);
    pthread_cond_destroy(&p_shared_int_cond_produce);
    pthread_mutex_destroy(&p_shared_int_mutex);
    delete p_shared_int;
    delete p_shared_int_delivery_status;
    pthread_barrier_destroy(&consumer_barrier);
    return sum;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        std::cerr << "Wrong number of args" << std::endl;
        exit(1);
    }

    N_consumer = (unsigned int) std::strtol(argv[1], nullptr, 10);
    milliseconds = (unsigned int) std::strtol(argv[2], nullptr, 10);

    std::cout << run_threads() << std::endl;
    return 0;
}