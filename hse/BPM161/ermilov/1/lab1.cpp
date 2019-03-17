#include <iostream>
#include <sstream>
#include <string>
#include <random>

#include <pthread.h>
#include "pthread_barrier.h"

struct config_t {
    int32_t N;
    int32_t sleep_time;
} config;

inline uint32_t random(uint32_t n) {
    static std::random_device rd;
    static std::mt19937 rnd(rd());
    static std::uniform_int_distribution<uint64_t> distr(0, 1ull * config.N * config.sleep_time - 1ull);
    return distr(rnd) % n;
}

bool empty_data = true;
bool finished = false;

std::vector<pthread_t> consumers;
thread_local int thread_sum;

pthread_mutex_t producer_cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumer_cond_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;
pthread_barrier_t threads_ready;

void* producer_routine(void* arg) {
    std::string input;
    std::getline(std::cin, input);
    std::istringstream in(input);

    pthread_barrier_wait(&threads_ready);

    int& value = *reinterpret_cast<int*>(arg);
    while (in >> value) {
        pthread_mutex_lock(&consumer_cond_mutex);
        empty_data = false;
        pthread_cond_signal(&consumer_cond);
        pthread_mutex_unlock(&consumer_cond_mutex);

        pthread_mutex_lock(&producer_cond_mutex);
        while (!empty_data) {
            pthread_cond_wait(&producer_cond, &producer_cond_mutex);
        }
        pthread_mutex_unlock(&producer_cond_mutex);
    }

    pthread_mutex_lock(&consumer_cond_mutex);
    finished = true;
    pthread_cond_broadcast(&consumer_cond);
    pthread_mutex_unlock(&consumer_cond_mutex);

    return nullptr;
}
 
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr); // ?
    pthread_barrier_wait(&threads_ready);

    thread_sum = 0;
    int& value = *reinterpret_cast<int*>(arg);
    while (!finished) {
        pthread_mutex_lock(&consumer_cond_mutex);
        while (!finished && empty_data) {
            pthread_cond_wait(&consumer_cond, &consumer_cond_mutex);
        }
        if (!finished) {
            thread_sum += value;
            pthread_mutex_lock(&producer_cond_mutex);
            empty_data = true;
            pthread_cond_signal(&producer_cond);
            pthread_mutex_unlock(&producer_cond_mutex);
        }
        pthread_mutex_unlock(&consumer_cond_mutex);

        uint32_t millis = random(config.sleep_time);
        nanosleep((const timespec[]){{millis / 1000, (millis % 1000) * 1000000}}, nullptr);
    }

    return new int(thread_sum);
}
 
void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&threads_ready);

    while (!finished) {
        auto& consumer = consumers[random(config.N)];
        pthread_cancel(consumer);
    }

    return nullptr;
}
 
int run_threads() {
    int shared_data;
    assert(pthread_barrier_init(&threads_ready, nullptr, config.N + 2) == 0);

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, &shared_data);

    consumers.resize(config.N);
    for (auto& consumer : consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, &shared_data);
    }

    pthread_t interrupter;
    pthread_create(&interrupter, nullptr, consumer_interruptor_routine, nullptr);

    int overall_sum = 0;
    for (auto& consumer : consumers) {
        int* thread_result;
        pthread_join(consumer, reinterpret_cast<void**>(&thread_result));
        overall_sum += *thread_result;
        delete thread_result;
    }
    
    return overall_sum;
}

int main(int argc, const char** argv) {
    assert(argc == 3);

    config.N = atoi(argv[1]);
    config.sleep_time = atoi(argv[2]) + 1;
    
    assert(config.N >= 0);
    assert(config.sleep_time >= 1);

    std::cout << run_threads() << std::endl;
    return 0;
}

