#include <pthread.h>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <vector>
#include <iterator>
#include <random>

thread_local int sum = 0;

int N;
int sleep_limit;
bool consumers_are_ready = false;
int ready_consumers = 0;
bool finished_job = false;
pthread_t* consumer_treads;

pthread_mutex_t consumers_are_ready_lock;
pthread_mutex_t new_value_available_lock;
pthread_mutex_t addition_is_completed_lock;

pthread_cond_t consumers_are_ready_cond;
pthread_cond_t new_value_available_cond;
pthread_cond_t addition_is_completed_cond;

bool logging_on = false;
void log(const std::string &str) {
    if (logging_on)
        std::cout << str << std::endl;
}

int intRand(const int & min, const int & max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<int> distribution(min,max);
    return distribution(generator);
}

inline void wait_for_consumers() {
    pthread_mutex_lock(&consumers_are_ready_lock);
    if (!consumers_are_ready) {
        pthread_cond_wait(&consumers_are_ready_cond, &consumers_are_ready_lock);
    }
    pthread_mutex_unlock(&consumers_are_ready_lock);
}

void* producer_routine(void* arg) {
    log("started producer");
    // Wait for consumer to start
    wait_for_consumers();
    log("Producer: consumers are ready");
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string line;
    getline(std::cin, line);
    std::istringstream iss(line);
    std::vector<int> numbers(std::istream_iterator<int>(iss), std::istream_iterator<int>{});
    for (int number : numbers) {
        log("sending value: " + std::to_string(number));
        pthread_mutex_lock(&addition_is_completed_lock);
        *(static_cast<int*>(arg)) = number;
        pthread_cond_signal(&new_value_available_cond);
        pthread_cond_wait(&addition_is_completed_cond, &addition_is_completed_lock);
        log("value " + std::to_string(number) + " was handled");
        pthread_mutex_unlock(&addition_is_completed_lock);
    }
    pthread_mutex_lock(&addition_is_completed_lock);
    finished_job = true;
    log("Producer: broadcasting");
    pthread_cond_broadcast(&new_value_available_cond);
    pthread_mutex_unlock(&addition_is_completed_lock);
}

void* consumer_routine(void* arg) {
    int id;

    log("Starting consumer");
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    // notify about start
    pthread_mutex_lock(&consumers_are_ready_lock);
    id = ready_consumers++;
    if (ready_consumers == N) {
        pthread_cond_broadcast(&consumers_are_ready_cond);
    }
    consumers_are_ready = true;
    pthread_mutex_unlock(&consumers_are_ready_lock);
    log("initialised consumer: " + std::to_string(id));

    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)
    while(!finished_job) {
        pthread_mutex_lock(&new_value_available_lock);
        if (finished_job) {
            break;
        }

        log("waiting consumer: " + std::to_string(id));
        pthread_cond_wait(&new_value_available_cond, &new_value_available_lock);
        if (finished_job) {
            break;
        }
        int value = *static_cast<int*>(arg);
        log("consumer " + std::to_string(id) + " received value " + std::to_string(value));
        pthread_cond_signal(&addition_is_completed_cond);
        pthread_mutex_unlock(&new_value_available_lock);
        sum += value;
        unsigned int time_to_sleep = static_cast<unsigned int>(intRand(0, sleep_limit));
        log("consumer " + std::to_string(id) + " sleeping for " + std::to_string(time_to_sleep));
        sleep(time_to_sleep);
        log("consumer " + std::to_string(id) + " awakened");
    }

    log("exiting consumer " + std::to_string(id));
    auto * sum_var = new int;
    *sum_var = sum;
    return sum_var;
}

void* consumer_interruptor_routine(void*) {
    log("started interruptor");
    // wait for consumers to start
    wait_for_consumers();
    log("interruptor: consumers are ready");
    while (!finished_job) {
        int consumer_to_interrupt = intRand(0, N - 1);
        //log("interrupting consumer " + std::to_string(consumer_to_interrupt));
        pthread_cancel(consumer_treads[consumer_to_interrupt]);
    }
    log("exiting interruptor");
    // interrupt random consumer while producer is running
}

void init_thread_variables() {
    pthread_cond_init(&new_value_available_cond, nullptr);
    pthread_cond_init(&addition_is_completed_cond, nullptr);
    pthread_cond_init(&consumers_are_ready_cond, nullptr);

    pthread_mutex_init(&new_value_available_lock, nullptr);
    pthread_mutex_init(&addition_is_completed_lock, nullptr);
    pthread_mutex_init(&consumers_are_ready_lock, nullptr);
}

void destroy_thread_variables() {
    pthread_cond_destroy(&new_value_available_cond);
    pthread_cond_destroy(&addition_is_completed_cond);
    pthread_cond_destroy(&consumers_are_ready_cond);

    pthread_mutex_destroy(&new_value_available_lock);
    pthread_mutex_destroy(&addition_is_completed_lock);
    pthread_mutex_destroy(&consumers_are_ready_lock);
}

int run_threads() {
    log("initializing");

    int value_storage;

    log("creating consumers");
    init_thread_variables();
    consumer_treads = new pthread_t[N];
    for (int i = 0; i < N; i++) {
        pthread_create(&consumer_treads[i], nullptr, consumer_routine, &value_storage);
    }

    // start N threads and wait until they're done
    pthread_t interruptor, producer;
    log("creating interruptor");
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, nullptr);
    log("creating producer");
    pthread_create(&producer, nullptr, producer_routine, &value_storage);

    log("waiting for producer");
    pthread_join(producer, nullptr);
    log("waiting for interruptor");
    pthread_join(interruptor, nullptr);

    int *consumer_sum;

    log("waiting for consumers");
    for (int i = 0; i < N; i++) {
        pthread_cond_broadcast(&new_value_available_cond);
        pthread_join(consumer_treads[i], reinterpret_cast<void **>(&consumer_sum));
        sum += *consumer_sum;
        delete consumer_sum;
    }
    log("deleting variables");

    destroy_thread_variables();
    delete [] consumer_treads;
    // return aggregated sum of values
    return sum;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cout << "Usage: numberOfThreads consumerSleepTime(ms)";
        return 1;
    }
    char * pEnd;
    N = static_cast<int>(strtol(argv[1], &pEnd, 10)) + 1;
    sleep_limit = static_cast<int>(strtol(argv[2], &pEnd, 10));

    std::cout << run_threads() << std::endl;
    return 0;
}