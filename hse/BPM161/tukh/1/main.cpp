#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <iterator>
#include <random>

#include <pthread.h>
#include <time.h>


struct common_threads_value {
    int value = 0;
    volatile bool has_value = false;
    volatile bool value_processed = false;

    pthread_cond_t value_available = PTHREAD_COND_INITIALIZER;
    pthread_cond_t value_process_done = PTHREAD_COND_INITIALIZER;

    pthread_mutex_t common_value_mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;


    ~common_threads_value() {
        pthread_mutex_destroy(&common_value_mutex);
        pthread_mutex_destroy(&producer_mutex);

        pthread_cond_destroy(&value_available);
        pthread_cond_destroy(&value_process_done);
    }
};

std::vector<pthread_t> consumer_threads;
pthread_barrier_t start_barrier;
std::size_t consumers_number;
unsigned int upper_sleep_threshold;
volatile bool stop = false;
__thread int sum = 0;

void read_numbers(std::vector<int>* destination) {
    // Reads a line from a standard input, adds all the ints in it to the destination vector.
    std::string line;
    std::getline(std::cin, line);
    std::istringstream str_stream(line);

    int number;
    while (str_stream >> number) {
        destination->push_back(number);
    }
}

void* producer_routine(void* arg) {
    // Wait for consumer to start

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    pthread_barrier_wait(&start_barrier);

    auto* value = (common_threads_value*) arg;

    std::vector<int> numbers;
    read_numbers(&numbers);

    for (auto number : numbers) {
        pthread_mutex_lock(&(value->common_value_mutex));
        value->value = number;
        value->has_value = true;
        pthread_cond_signal(&(value->value_available));
        pthread_mutex_unlock(&(value->common_value_mutex));

        pthread_mutex_lock(&(value->producer_mutex));
        while (!value->value_processed) {
            pthread_cond_wait(&(value->value_process_done), &(value->producer_mutex));
        }
        value->value_processed = false;
        pthread_mutex_unlock(&(value->producer_mutex));
    }

    pthread_mutex_lock(&(value->common_value_mutex));
    stop = true;
    pthread_cond_broadcast(&(value->value_available));
    pthread_mutex_unlock(&(value->common_value_mutex));

    return nullptr;
}

void* consumer_routine(void* arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&start_barrier);

    std::mt19937 rnd;
    rnd.seed(static_cast<unsigned long>(time(nullptr)));

    auto* value = (common_threads_value*) arg;


    while (true) {
        pthread_mutex_lock(&(value->common_value_mutex));
        while (!stop && !value->has_value) {
            pthread_cond_wait(&(value->value_available), &(value->common_value_mutex));
        }
        if (stop) {
            pthread_mutex_unlock(&(value->common_value_mutex));
            break;
        }
        sum += value->value;
        value->has_value = false;
        pthread_mutex_unlock(&(value->common_value_mutex));

        pthread_mutex_lock(&(value->producer_mutex));
        value->value_processed = true;
        pthread_cond_signal(&(value->value_process_done));
        pthread_mutex_unlock(&(value->producer_mutex));

        long time_to_sleep_nano = 1000000 * (rnd() % (upper_sleep_threshold + 1));
        timespec sleep_time = {0, time_to_sleep_nano};
        nanosleep(&sleep_time, {});
    }

    return &sum;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start

    // interrupt random consumer while producer is running

    pthread_barrier_wait(&start_barrier);

    auto value = (common_threads_value*) arg;

    std::mt19937 rnd;
    rnd.seed(static_cast<unsigned long>(time(nullptr)));

    while (true) {
        // Actually in each place in program where we deal with stop, we also have
        // common_value_mutex lock. Maybe it's better (in logical meaning) to create two separate
        // mutexes, one for value from common_threads_value and one for stop. But we will not.

        pthread_mutex_lock(&(value->common_value_mutex));
        if (stop) {
            pthread_mutex_unlock(&(value->common_value_mutex));
            break;
        }
        pthread_mutex_unlock(&(value->common_value_mutex));

        pthread_t& victim = consumer_threads[rnd() % consumer_threads.size()];
        pthread_cancel(victim);
    }

    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values

    pthread_barrier_init(&start_barrier, nullptr, static_cast<unsigned int>(consumers_number + 2));
    common_threads_value value;

    pthread_t interruptor_thread;
    pthread_create(&interruptor_thread, nullptr, consumer_interruptor_routine, &value);

    consumer_threads.resize(consumers_number);
    for (auto& thread : consumer_threads) {
        pthread_create(&thread, nullptr, consumer_routine, &value);
    }

    pthread_t producer_thread;
    pthread_create(&producer_thread, nullptr, producer_routine, &value);

    pthread_join(producer_thread, nullptr);
    pthread_join(interruptor_thread, nullptr);

    pthread_barrier_destroy(&start_barrier);

    int sum = 0;
    int* thread_value;
    for (auto& thread : consumer_threads) {
        pthread_join(thread, (void**) &thread_value);
        sum += *thread_value;
    }

    return sum;
}

int main(int argc, char** argv) {
    if (argc != 3) {
        std::cout << "Usage: please provide exactly two arguments, number of the consumer threads "
                     "and upper sleep threshold.";
        return 0;
    }

    consumers_number = static_cast<std::size_t>(std::atoi(argv[1]));
    upper_sleep_threshold = static_cast<unsigned int>(std::atoi(argv[2]));
    std::cout << run_threads() << std::endl;
    return 0;
}