#include <iostream>
#include <vector>
#include <pthread.h>
#include <sstream>
#include <unistd.h>
#include <numeric>
#include <random>

int NUM_CONSUMERS = 1;
int MAX_SLEEP_MS = 0;

int CURRENT_NUMBER = -1;
bool NUMBERS_LEFT = true;

struct consumer_arg {
    const int * current_number_ptr = &CURRENT_NUMBER;
    int * sum_ptr = nullptr;
    explicit consumer_arg(int * sum_ptr) {
        this->sum_ptr = sum_ptr;
    }
    consumer_arg() = default;
};

bool AT_LEAST_ONE_THREAD_STARTED = false;
int NUM_CONSUMER_THREADS_SET_NO_INTERRUPT = 0;
pthread_mutex_t AT_LEAST_ONE_THREAD_STARTED_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t AT_LEAST_ONE_THREAD_STARTED_COND = PTHREAD_COND_INITIALIZER;

bool IS_NUMBER_CONSUMED = true;
pthread_mutex_t CURRENT_NUMBER_MUTEX = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t NUMBER_PRODUCED_COND = PTHREAD_COND_INITIALIZER;
pthread_cond_t NUMBER_CONSUMED_COND = PTHREAD_COND_INITIALIZER;

std::vector<pthread_t> THREADS;


std::vector<int> read_data() {
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

    if (!AT_LEAST_ONE_THREAD_STARTED) {
        pthread_mutex_lock(&AT_LEAST_ONE_THREAD_STARTED_MUTEX);
        if (!AT_LEAST_ONE_THREAD_STARTED) {
            pthread_cond_wait(&AT_LEAST_ONE_THREAD_STARTED_COND, &AT_LEAST_ONE_THREAD_STARTED_MUTEX);
        }
        pthread_mutex_unlock(&AT_LEAST_ONE_THREAD_STARTED_MUTEX);
    }

    for (int i = 0; i < numbers.size(); i++) {
        pthread_mutex_lock(&CURRENT_NUMBER_MUTEX);
        assert(IS_NUMBER_CONSUMED);
        CURRENT_NUMBER = numbers[i];
        IS_NUMBER_CONSUMED = false;
        // проверка, что хоть кто-то ждет не нужна, т.к. есть флаг скушано ли число
        pthread_cond_signal(&NUMBER_PRODUCED_COND);
        pthread_cond_wait(&NUMBER_CONSUMED_COND, &CURRENT_NUMBER_MUTEX);
        assert(IS_NUMBER_CONSUMED);
        if (i == numbers.size() - 1) {
            NUMBERS_LEFT = false;
            pthread_cond_broadcast(&NUMBER_PRODUCED_COND); // can this be outside of the mutex?
            // wakes up all consumers so that they'll found out that no numbers left and terminate
        }
        pthread_mutex_unlock(&CURRENT_NUMBER_MUTEX);
    }

    return nullptr;
}

void* consumer_routine(void* arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_mutex_lock(&AT_LEAST_ONE_THREAD_STARTED_MUTEX);
    AT_LEAST_ONE_THREAD_STARTED = true;
    NUM_CONSUMER_THREADS_SET_NO_INTERRUPT++;
    pthread_cond_signal(&AT_LEAST_ONE_THREAD_STARTED_COND);
    pthread_mutex_unlock(&AT_LEAST_ONE_THREAD_STARTED_MUTEX);

    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, MAX_SLEEP_MS); // []

    while (NUMBERS_LEFT) {
        pthread_mutex_lock(&CURRENT_NUMBER_MUTEX);
        if (!NUMBERS_LEFT) {
            pthread_mutex_unlock(&CURRENT_NUMBER_MUTEX);
            break;
        }
        if (IS_NUMBER_CONSUMED) {
            pthread_cond_wait(&NUMBER_PRODUCED_COND, &CURRENT_NUMBER_MUTEX);
            if (IS_NUMBER_CONSUMED) {
                pthread_mutex_unlock(&CURRENT_NUMBER_MUTEX);
                continue;
            }
        }
        *(((consumer_arg *)arg)->sum_ptr) += *(((consumer_arg *)arg)->current_number_ptr);
        IS_NUMBER_CONSUMED = true;
        pthread_cond_signal(&NUMBER_CONSUMED_COND);
        pthread_mutex_unlock(&CURRENT_NUMBER_MUTEX);

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
    std::uniform_int_distribution<std::mt19937::result_type> dist(0, NUM_CONSUMERS - 1); // []

    while (NUMBERS_LEFT) {
        int random_consumer_thread_index = dist(rng);
        pthread_cancel(THREADS[random_consumer_thread_index]);
    }

    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values

    std::vector<int> sums(NUM_CONSUMERS, 0);
    std::vector<consumer_arg> CONSUMER_ARGS(NUM_CONSUMERS);
    THREADS.resize(NUM_CONSUMERS + 2);

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        CONSUMER_ARGS[i] = consumer_arg(&(sums[i]));
    }

    for (int i = 0; i < NUM_CONSUMERS; i++) {
        pthread_create(&THREADS[i], nullptr, consumer_routine, &(CONSUMER_ARGS[i]));
    }
    pthread_create(&THREADS[NUM_CONSUMERS], nullptr, producer_routine, nullptr);

    while (NUM_CONSUMER_THREADS_SET_NO_INTERRUPT != NUM_CONSUMERS) {
        pthread_create(&THREADS[NUM_CONSUMERS + 1], nullptr, consumer_interruptor_routine, nullptr);
    }

    for (pthread_t thread : THREADS) {
        pthread_join(thread, nullptr);
    }

    return std::accumulate(sums.begin(), sums.end(), 0);
}

int main(int argc, char *argv[]) {
    NUM_CONSUMERS = atoi(argv[1]);
    MAX_SLEEP_MS = atoi(argv[2]);

    std::cout << run_threads() << std::endl;
    return 0;
}