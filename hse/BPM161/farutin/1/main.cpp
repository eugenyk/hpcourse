#include <pthread.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <time.h>

typedef struct {
    pthread_barrier_t *start_sync_barrier;

    pthread_mutex_t *value_mutex;
    pthread_cond_t *value_updated;
    pthread_cond_t *value_processed;
    int *value;
    bool *is_value_actual;
    bool *producer_finished;
} producer_arg_t;

typedef struct {
    uint32_t max_sleep_time;

    pthread_barrier_t *start_sync_barrier;

    pthread_mutex_t *value_mutex;
    pthread_cond_t *value_updated;
    pthread_cond_t *value_processed;
    int *value;
    bool *is_value_actual;
    bool *producer_finished;
} consumer_arg_t;

typedef struct {
    std::vector<pthread_t> *consumer_threads;

    pthread_barrier_t *start_sync_barrier;

    pthread_mutex_t *value_mutex;
    bool *producer_finished;
} interrupter_arg_t;
 
void* producer_routine(void* arg) {
    producer_arg_t *producer_arg = (producer_arg_t *)arg;
    int rc = pthread_barrier_wait(producer_arg->start_sync_barrier);
    if (rc != PTHREAD_BARRIER_SERIAL_THREAD && rc != 0) {
        std::cerr << "failed to create start synchronization barrier" << std::endl;
        exit(0);
    }

    std::vector<int> numbers;
    std::string data;
    std::getline(std::cin, data);
    std::istringstream sin(data);
    int n;

    while (sin >> n) {
        numbers.push_back(n);
    }
    
    for (size_t i = 0; i < numbers.size(); i++) {
        pthread_mutex_lock(producer_arg->value_mutex);

        *producer_arg->value = numbers[i];
        *producer_arg->is_value_actual = true;
        pthread_cond_signal(producer_arg->value_updated);

        while (*producer_arg->is_value_actual) {
            pthread_cond_wait(producer_arg->value_processed, producer_arg->value_mutex);
        }
        
        if (i == numbers.size() - 1) {
            *producer_arg->producer_finished = true;
            pthread_cond_broadcast(producer_arg->value_updated);
        }
        
        pthread_mutex_unlock(producer_arg->value_mutex);
    }
}
 
void* consumer_routine(void* arg) {
    static __thread int64_t part_sum;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    
    consumer_arg_t *consumer_arg = (consumer_arg_t *)arg;
    int rc = pthread_barrier_wait(consumer_arg->start_sync_barrier);
    if (rc != PTHREAD_BARRIER_SERIAL_THREAD && rc != 0) {
        std::cerr << "failed to create start synchronization barrier" << std::endl;
        exit(0);
    }

    while (true) {
        pthread_mutex_lock(consumer_arg->value_mutex);

        while (!*consumer_arg->is_value_actual) {
            if (*consumer_arg->producer_finished) {
                pthread_mutex_unlock(consumer_arg->value_mutex);
                pthread_exit(&part_sum);
            }

            pthread_cond_wait(consumer_arg->value_updated, consumer_arg->value_mutex);
        }

        part_sum += *consumer_arg->value;
        *consumer_arg->is_value_actual = false;
        pthread_cond_signal(consumer_arg->value_processed);
 
        pthread_mutex_unlock(consumer_arg->value_mutex);
        
        if (consumer_arg->max_sleep_time != 0) {
            timespec sleepTimeSpec = {0, (rand() % consumer_arg->max_sleep_time) * 1000000};
            nanosleep(&sleepTimeSpec, nullptr);
        }
    }
}
 
void* consumer_interruptor_routine(void* arg) {
    interrupter_arg_t *interrupter_arg = (interrupter_arg_t *)arg;
    int rc = pthread_barrier_wait(interrupter_arg->start_sync_barrier);
    if (rc != PTHREAD_BARRIER_SERIAL_THREAD && rc != 0) {
        std::cerr << "failed to create start synchronization barrier" << std::endl;
        exit(0);
    }

    std::vector<pthread_t> *consumer_threads = interrupter_arg->consumer_threads;
    size_t consumer_cnt = consumer_threads->size();
    
    while (true) {
        pthread_mutex_lock(interrupter_arg->value_mutex);

        if (*interrupter_arg->producer_finished) {
            pthread_mutex_unlock(interrupter_arg->value_mutex);
            break;
        }

        pthread_mutex_unlock(interrupter_arg->value_mutex);
        
        size_t idx = rand() % consumer_cnt;
        pthread_cancel((*consumer_threads)[idx]);
    }
}
 
int run_threads(uint32_t consumer_number, uint32_t consumer_max_sleep_time) {
    int rc;

    pthread_barrier_t start_sync_barrier;
    rc = pthread_barrier_init(&start_sync_barrier, nullptr, consumer_number + 2);
    if (rc) {
        std::cerr << "failed to create start synchronization barrier" << std::endl;
        exit(0);
    }
    
    pthread_mutex_t value_mutex;
    rc = pthread_mutex_init(&value_mutex, nullptr);
    if (rc) {
        std::cerr << "failed to create value mutex" << std::endl;
        exit(0);        
    }

    pthread_cond_t value_updated;
    rc = pthread_cond_init(&value_updated, nullptr);
    if (rc) {
        std::cerr << "failed to create condition variable" << std::endl;
        exit(0);        
    }
    
    pthread_cond_t value_processed;
    rc = pthread_cond_init(&value_processed, nullptr);
    if (rc) {
        std::cerr << "failed to create condition variable" << std::endl;
        exit(0);        
    }

    bool producer_finished = false;
    int value = 0;
    bool is_value_actual = false;

    producer_arg_t producer_arg;
    producer_arg.start_sync_barrier = &start_sync_barrier;
    producer_arg.value_mutex = &value_mutex;
    producer_arg.value_updated = &value_updated;
    producer_arg.value_processed = &value_processed;
    producer_arg.value = &value;
    producer_arg.is_value_actual = &is_value_actual;
    producer_arg.producer_finished = &producer_finished;
    
    pthread_t producer_thread;
    rc = pthread_create(&producer_thread, nullptr, producer_routine, &producer_arg);
    if (rc) {
        std::cerr << "failed to create producer thread" << std::endl;
        exit(0);
    }
    
    consumer_arg_t consumer_arg;
    consumer_arg.max_sleep_time = consumer_max_sleep_time;
    consumer_arg.start_sync_barrier = &start_sync_barrier;
    consumer_arg.value_mutex = &value_mutex;
    consumer_arg.value_updated = &value_updated;
    consumer_arg.value_processed = &value_processed;
    consumer_arg.value = &value;
    consumer_arg.is_value_actual = &is_value_actual;
    consumer_arg.producer_finished = &producer_finished;

    std::vector<pthread_t> consumer_threads(consumer_number);
    
    for (pthread_t &consumer_thread : consumer_threads) {
        rc = pthread_create(&consumer_thread, nullptr, consumer_routine, &consumer_arg);
        if (rc) {
            std::cerr << "failed to create consumer thread" << std::endl;
            exit(0);
        }        
    }
    
    interrupter_arg_t interrupter_arg;
    interrupter_arg.consumer_threads = &consumer_threads;
    interrupter_arg.start_sync_barrier = &start_sync_barrier;
    interrupter_arg.value_mutex = &value_mutex;
    interrupter_arg.producer_finished = &producer_finished;

    pthread_t interrupter_thread;
    rc = pthread_create(&interrupter_thread, nullptr, consumer_interruptor_routine, &interrupter_arg);
    if (rc) {
        std::cerr << "failed to create interrupter thread" << std::endl;
        exit(0);
    }
        
    int64_t sum = 0;
    int64_t *result;

    pthread_join(interrupter_thread, nullptr);
    
    for (pthread_t &consumer_thread : consumer_threads) {
        pthread_join(consumer_thread, (void **) &result);
        sum += *result;
    }

    pthread_join(producer_thread, nullptr);

    pthread_barrier_destroy(&start_sync_barrier);
    
    return sum;
}
 
int main(int argc, char *argv[]) {
    if (argc < 3) {
        fprintf(stderr, "usage: %s <consumer number> <consumer max sleep time, ms>\n", argv[0]);
        return 0;
    }
    
    uint32_t consumer_number = static_cast<uint32_t>(strtol(argv[1], nullptr, 10));
    uint32_t consumer_max_sleep_time = static_cast<uint32_t>(strtol(argv[2], nullptr, 10));
    
    std::cout << run_threads(consumer_number, consumer_max_sleep_time) << std::endl;
    return 0;
}

