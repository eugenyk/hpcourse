#include <iostream>
#include <vector>
#include <pthread.h>
#include <sstream>
#include <unistd.h>
#include <numeric>
#include <random>

int current_number = -1;

struct consumer_arg {
    const int * current_number_ptr = &current_number;
    int * sum_ptr = nullptr;
    explicit consumer_arg(int * sum_ptr) {
        this->sum_ptr = sum_ptr;
    }
    consumer_arg() = default;
};

int num_consumers = 1;
int max_sleep_ms = 0;
bool are_numbers_left = true;
bool is_number_consumed = true;
pthread_mutex_t current_number_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t number_produced_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t number_consumed_cond = PTHREAD_COND_INITIALIZER;
std::vector<pthread_t> threads;


std::vector<int> read_data() {
    return {7, 8, 9, 10, 11};
    std::vector<int> numbers;
    std::string line;
    std::getline(std::cin, line);
    std::istringstream iss(line);
    while (!iss.eof()) {
        int number;
        iss >> number;
        numbers.push_back(number);
    }
    return numbers;
}

void* producer_routine(void* arg) {
    // Wait for consumer to start
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    std::vector<int> numbers = read_data();
    assert(!numbers.empty());

    for (unsigned long i = 0; i < numbers.size(); i++) {
        pthread_mutex_lock(&current_number_mutex);
        current_number = numbers[i];
        is_number_consumed = false;
        pthread_cond_signal(&number_produced_cond);
        while (!is_number_consumed) {
            pthread_cond_wait(&number_consumed_cond, &current_number_mutex);
        }
        if (i == numbers.size() - 1) {
            are_numbers_left = false;
            // wake up all consumers, they'll know that no numbers left and terminate
            pthread_cond_broadcast(&number_produced_cond);
        }
        pthread_mutex_unlock(&current_number_mutex);
    }

    return nullptr;
}

void* consumer_routine(void* arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, max_sleep_ms); // []

    while (are_numbers_left) {
        pthread_mutex_lock(&current_number_mutex);
        while (are_numbers_left && is_number_consumed) {
            pthread_cond_wait(&number_produced_cond, &current_number_mutex);
        }
        if (!are_numbers_left) {
            pthread_mutex_unlock(&current_number_mutex);
            break;
        }

        *(((consumer_arg *)arg)->sum_ptr) += *(((consumer_arg *)arg)->current_number_ptr);
        is_number_consumed = true;
        pthread_cond_signal(&number_consumed_cond);
        pthread_mutex_unlock(&current_number_mutex);

        int sleep_time = dist(rng);

        usleep(sleep_time * 1000);
    }
    return nullptr;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start
    // interrupt random consumer while producer is running

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, num_consumers - 1); // []

    while (are_numbers_left) {
        int random_consumer_thread_index = dist(rng);
        pthread_cancel(threads[random_consumer_thread_index]);
    }

    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values

    std::vector<int> sums(num_consumers, 0);
    std::vector<consumer_arg> consumer_args(num_consumers);
    threads.resize(num_consumers + 2);

    for (int i = 0; i < num_consumers; i++) {
        consumer_args[i] = consumer_arg(&(sums[i]));
        pthread_create(&threads[i], nullptr, consumer_routine, &(consumer_args[i]));
    }
    pthread_create(&threads[num_consumers], nullptr, producer_routine, nullptr);
    pthread_create(&threads[num_consumers + 1], nullptr, consumer_interruptor_routine, nullptr);

    for (pthread_t thread : threads) {
        pthread_join(thread, nullptr);
    }

    return std::accumulate(sums.begin(), sums.end(), 0);
}

int main(int argc, char *argv[]) {
    num_consumers = atoi(argv[1]);
    max_sleep_ms = atoi(argv[2]);

    std::cout << run_threads() << std::endl;
    return 0;
}