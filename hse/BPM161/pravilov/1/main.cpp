#include <random>

#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <unistd.h>
#include <algorithm>
#include <cstring>
#include <time.h>

unsigned int max_sleep_time;
unsigned int N;
enum STATUS {
    EMPTY, FILLED, NO_DATA
};
STATUS status = EMPTY;
int value;
pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t value_updated = PTHREAD_COND_INITIALIZER;
pthread_cond_t value_taken = PTHREAD_COND_INITIALIZER;
pthread_barrier_t barrier;
thread_local int partial_sum;

template <class T>
T thread_safe_rand(const T &min, const T &max) {
    static thread_local std::mt19937 generator;
    std::uniform_int_distribution<T> distribution(min,max);
    return distribution(generator);
}

std::vector<int> read_integers() {
    std::string input;
    std::getline(std::cin, input);
    std::stringstream stream(input);
    std::vector<int> integers;
    while(!stream.eof()) {
        int n;
        stream >> n;
        integers.push_back(n);
    }
    return integers;
}

// Wait for consumer to start
// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
void* producer_routine(void* arg) {
    std::vector<int> integers = read_integers();
    pthread_barrier_wait(&barrier);
    for (auto integer : integers) {
        pthread_mutex_lock(&value_mutex);
        while (status != EMPTY) {
            pthread_cond_wait(&value_taken, &value_mutex);
        }
        value = integer;
        status = FILLED;
        pthread_cond_signal(&value_updated);
        pthread_mutex_unlock(&value_mutex);
    }
    pthread_mutex_lock(&value_mutex);
    while (status != EMPTY) {
        pthread_cond_wait(&value_taken, &value_mutex);
    }
    status = NO_DATA;
    pthread_cond_broadcast(&value_updated);
    pthread_mutex_unlock(&value_mutex);
    return nullptr;
}

// notify about start
// for every update issued by producer, read the value and add to sum
// return pointer to result (for particular consumer)
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&barrier);
    auto value_shared = (int *) arg;
    pthread_mutex_lock(&value_mutex);
    STATUS local_status = status;
    pthread_mutex_unlock(&value_mutex);
    while (local_status != NO_DATA) {
        pthread_mutex_lock(&value_mutex);
        while (status == EMPTY) {
            pthread_cond_wait(&value_updated, &value_mutex);
        }
        if (status == FILLED) {
            partial_sum += *value_shared;
            status = EMPTY;
            pthread_cond_signal(&value_taken);
        }
        local_status = status;
        pthread_mutex_unlock(&value_mutex);
        struct timespec sleep_time{};
        sleep_time.tv_sec = 0;
        sleep_time.tv_nsec = thread_safe_rand<unsigned int>(0, max_sleep_time);
        if (nanosleep(&sleep_time, NULL) < 0) {
            std::cerr << "Thread cannot sleep";
        }
    }
    // pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr); I think it is not needed
    return &partial_sum;
}

// wait for consumers to start
// interrupt random consumer while producer is running
void* consumer_interruptor_routine(void* arg) {
    auto threads_ptr = (pthread_t *) arg;
    auto threads = std::vector<pthread_t>(threads_ptr, threads_ptr + N);
    pthread_barrier_wait(&barrier);
    while(true) {
        pthread_mutex_lock(&value_mutex);
        if (status == NO_DATA) {
            pthread_mutex_unlock(&value_mutex);
            return nullptr;
        }
        pthread_mutex_unlock(&value_mutex);
        pthread_cancel(threads[thread_safe_rand<size_t>(0, threads.size() - 1)]);
    }
}

void check_error(int ret, const std::string &action) {
    if (ret != 0) {
        std::cerr << "error: " << strerror(ret) << ". Action: " << action;
        exit(1);
    }
}

// start N threads and wait until they're done
// return aggregated sum of values
int run_threads() {
    unsigned int num_barriers = N + 2;
    pthread_barrier_init(&barrier, nullptr, num_barriers);

    pthread_t producer, interruptor;
    pthread_t consumers[N];
    check_error(pthread_create(&producer, nullptr, producer_routine, nullptr), "pthread_create");
    for (int i = 0; i < N; i++) {
        check_error(pthread_create(&consumers[i], nullptr, consumer_routine, &value),
                "pthread_create producer #" + std::to_string(i));
    }
    check_error(pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers), "pthread_creeate interruptor");

    check_error(pthread_join(interruptor, nullptr), "pthread_join interruptor");
    int result = 0;
    for (int i = 0; i < N; i++) {
        void *ret;
        check_error(pthread_join(consumers[i], &ret), "pthread_join consumer #" + std::to_string(i));
        result += *((int *) ret);
    }
    check_error(pthread_join(producer, nullptr), "pthread_join producer");

    return result;
}

void parse_arguments_and_set_global_variables(int argc, char **argv) {
    if (argc != 3) {
        perror("Usage: <number of consumers> <sleep time>");
        exit(1);
    }
    N = (unsigned int) std::strtol(argv[1], nullptr, 10);
    max_sleep_time = (unsigned int) std::strtol(argv[2], nullptr, 10) * 1000000; // convert milliseconds to nanoseconds
}

int main(int argc, char **argv) {
    parse_arguments_and_set_global_variables(argc, argv);
    std::cout << run_threads() << std::endl;
    return 0;
}