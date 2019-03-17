#include <pthread.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <random>
#include <thread>

__thread int partial_sum = 0;
bool is_running = true;

unsigned int number_of_consumers;
int consumer_sleep_upper_limit = 0;
pthread_barrier_t consumers_barrier;
std::vector<pthread_t> consumers;

struct thread_primitives {
    pthread_mutex_t shared_variable_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t value_produced_condition = PTHREAD_COND_INITIALIZER;
    pthread_cond_t value_consumed_condition = PTHREAD_COND_INITIALIZER;
};

thread_primitives primitives;

int get_random_integer(int lower_limit, int upper_limit) {
    std::random_device dev;
    std::mt19937 generator(dev());
    std::uniform_int_distribution<int> distribution(lower_limit, upper_limit);
    return distribution(generator);
}

void* producer_routine(void* arg) {
    // Wait for consumer to start
    pthread_barrier_wait(&consumers_barrier);

    // Read primitives, loop through each value and update the value, notify consumer, wait for consumer to process
    int* shared_variable_pointer = static_cast<int*>(arg);
    std::string input;
    std::getline(std::cin, input);
    std::stringstream ss(input);

    int n;
    while (ss >> n) {
        pthread_mutex_lock(&primitives.shared_variable_mutex);
        *shared_variable_pointer = n;
        pthread_cond_signal(&primitives.value_produced_condition);
        pthread_mutex_unlock(&primitives.shared_variable_mutex);

        pthread_mutex_lock(&primitives.producer_mutex);    
        while (*shared_variable_pointer != 0) {
            pthread_cond_wait(&primitives.value_consumed_condition, &primitives.producer_mutex);
        }
        pthread_mutex_unlock(&primitives.producer_mutex);
    }

    pthread_mutex_lock(&primitives.shared_variable_mutex);
    is_running = false;
    pthread_cond_broadcast(&primitives.value_produced_condition);
    pthread_mutex_unlock(&primitives.shared_variable_mutex);

    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&consumers_barrier);

    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)
    int* shared_variable_pointer = static_cast<int*>(arg);
    while (is_running) {
        pthread_mutex_lock(&primitives.shared_variable_mutex);
        while (is_running && *shared_variable_pointer == 0) {
            pthread_cond_wait(&primitives.value_produced_condition, &primitives.shared_variable_mutex);
        }
        if (!is_running) {
            pthread_mutex_unlock(&primitives.shared_variable_mutex);
            break;
        }
        partial_sum += *shared_variable_pointer;
        *shared_variable_pointer = 0;

        pthread_mutex_lock(&primitives.producer_mutex);
        pthread_cond_signal(&primitives.value_consumed_condition);
        pthread_mutex_unlock(&primitives.producer_mutex);
        pthread_mutex_unlock(&primitives.shared_variable_mutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(get_random_integer(0, consumer_sleep_upper_limit)));
    }

    return &partial_sum;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start
    pthread_barrier_wait(&consumers_barrier);

    // interrupt random consumer while producer is running
    while (is_running) {
        pthread_cancel(consumers[get_random_integer(0, number_of_consumers - 1)]);
    }

    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values
    pthread_barrier_init(&consumers_barrier, nullptr, number_of_consumers + 2);

    int updates_variable = 0;

    pthread_t producer, interrupter;
    pthread_create(&producer, nullptr, producer_routine, &updates_variable);
    pthread_create(&interrupter, nullptr, consumer_interruptor_routine, nullptr);
    consumers = std::vector<pthread_t>(number_of_consumers);
    for (auto& consumer : consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, &updates_variable);
    }

    int total_sum = 0;
    for (auto& consumer : consumers) {
        int* return_value;
        pthread_join(consumer, (void **) &return_value);
        total_sum += *return_value;
    }

    return total_sum;
}

int main(int argc, char *argv[]) {
    number_of_consumers = static_cast<unsigned int>(std::stoi(argv[1]));
    consumer_sleep_upper_limit = std::stoi(argv[2]);

    std::cout << run_threads() << std::endl;
    return 0;
}


