#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>


struct value_store {
    int value = 0;
    pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t value_cond = PTHREAD_COND_INITIALIZER;

    pthread_mutex_t done_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t done_cond = PTHREAD_COND_INITIALIZER;
};

std::vector<pthread_t> consumers;
unsigned int sleep_time;
unsigned int consumers_number;

pthread_barrier_t all_threads_started;

__thread int accumulated = 0;
bool finished = false;

void start_sleep() {
    timespec sleepValue = {0};
    sleepValue.tv_nsec = (rand() % sleep_time) * 1000000;
    nanosleep(&sleepValue, nullptr);
}

void* producer_routine(void* arg) {
    // Wait for consumer to start
    pthread_barrier_wait(&all_threads_started);
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::string values;
    getline(std::cin, values);
    std::istringstream values_stream(values);

    value_store* shared_value = static_cast<value_store*>(arg);

    int value;
    while (values_stream >> value) {
        pthread_mutex_lock(&shared_value->value_mutex);
        shared_value->value = value;
        pthread_cond_signal(&shared_value->value_cond);
        pthread_mutex_unlock(&shared_value->value_mutex);

        pthread_mutex_lock(&shared_value->done_mutex);
        while (shared_value->value != 0) {
            pthread_cond_wait(&shared_value->done_cond, &shared_value->done_mutex);
        }
        pthread_mutex_unlock(&shared_value->done_mutex);
    }

    pthread_mutex_lock(&shared_value->value_mutex);
    finished = true;
    shared_value->value = 0;
    pthread_cond_broadcast(&shared_value->value_cond);
    pthread_mutex_unlock(&shared_value->value_mutex);
    return nullptr;
}

void* consumer_routine(void* arg) {
    int old_state;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
    // notify about start
    pthread_barrier_wait(&all_threads_started);
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)
    value_store* shared_value = static_cast<value_store*>(arg);
    while (!finished) {
        int value = 0;
        pthread_mutex_lock(&shared_value->value_mutex);
        while (shared_value->value == 0 && !finished) {
            pthread_cond_wait(&shared_value->value_cond, &shared_value->value_mutex);
        }
        value = shared_value->value;
        shared_value->value = 0;
        pthread_mutex_unlock(&shared_value->value_mutex);

        if (finished) {
            break;
        }

        accumulated += value;

        pthread_mutex_lock(&shared_value->done_mutex);
        pthread_cond_signal(&shared_value->done_cond);
        pthread_mutex_unlock(&shared_value->done_mutex);

        start_sleep();
    }
    return &accumulated;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start
    pthread_barrier_wait(&all_threads_started);
    // interrupt random consumer while producer is running
    while (!finished) {
        int index = rand() % consumers_number;
        pthread_cancel(consumers[index]);
    }
    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values
    srand(static_cast<unsigned int>(time(nullptr)));
    pthread_barrier_init(&all_threads_started, nullptr, consumers_number + 2);
    consumers.resize(consumers_number);

    value_store shared_value;

    for (auto& consumer : consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, &shared_value);
    }
    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, &shared_value);
    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, nullptr);

    int sum = 0;

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    for (auto& consumer : consumers) {
        int* value = nullptr;
        pthread_join(consumer, (void**) &value);
        sum += *value;
    }

    pthread_cond_destroy(&shared_value.done_cond);
    pthread_cond_destroy(&shared_value.value_cond);
    pthread_mutex_destroy(&shared_value.done_mutex);
    pthread_mutex_destroy(&shared_value.value_mutex);

    return sum;
}

int main(int argc, char** argv) {
    consumers_number = static_cast<unsigned int>(std::stoi(argv[1]));
    sleep_time = static_cast<unsigned int>(std::stoi(argv[2])) + 1;
    std::cout << run_threads() << std::endl;
    return 0;
}