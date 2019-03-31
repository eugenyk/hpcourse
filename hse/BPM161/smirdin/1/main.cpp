#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>

__thread int counter = 0;

std::vector<pthread_t> consumers;

bool finished = false;

unsigned int sleep_time;

pthread_mutex_t shared_variable_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t consumer_notification = PTHREAD_COND_INITIALIZER;
pthread_cond_t producer_notification = PTHREAD_COND_INITIALIZER;

pthread_barrier_t all_threads_started;

const int NANOS_IN_MILLI = 1000000;

void* producer_routine(void* arg) {
    pthread_barrier_wait(&all_threads_started);

    int* number = static_cast<int*>(arg);
    std::string input;
    std::getline(std::cin, input);
    std::stringstream sin(input);
    while (sin >> *number) {
        pthread_mutex_lock(&shared_variable_mutex);
        pthread_cond_signal(&consumer_notification);

        while (*number != 0) {
            pthread_cond_wait(&producer_notification, &shared_variable_mutex);
        }

        pthread_mutex_unlock(&shared_variable_mutex);
    }

    pthread_mutex_lock(&shared_variable_mutex);
    finished = true;

    pthread_cond_broadcast(&consumer_notification);
    pthread_mutex_unlock(&shared_variable_mutex);
    return nullptr;
}


void* consumer_routine(void* arg) {
    int old_state;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &old_state);
    pthread_barrier_wait(&all_threads_started);

    auto number = static_cast<int*>(arg);

    while (!finished) {
        pthread_mutex_lock(&shared_variable_mutex);
        while (!finished && *number == 0) {
            pthread_cond_wait(&consumer_notification, &shared_variable_mutex);
        }
        if (finished) {
            pthread_mutex_unlock(&shared_variable_mutex);
            break;
        }
        counter += *number;
        *number = 0;

        pthread_cond_signal(&producer_notification);
        pthread_mutex_unlock(&shared_variable_mutex);

        timespec sleepValue = {0};
        sleepValue.tv_nsec = (rand() % sleep_time) * NANOS_IN_MILLI;
        nanosleep(&sleepValue, nullptr);
    }
    auto result = new int(counter);
    return result;
}

void* consumer_interrupter_routine(void* arg) {
    pthread_barrier_wait(&all_threads_started);
    while (true) {
        pthread_mutex_lock(&shared_variable_mutex);
        if (finished) {
            pthread_mutex_unlock(&shared_variable_mutex);
            return nullptr;
        }
        pthread_cancel(consumers[rand() % consumers.size()]);
        pthread_mutex_unlock(&shared_variable_mutex);
    }
}

int run_threads(unsigned int N) {
    srand(time(nullptr));
    pthread_barrier_init(&all_threads_started, nullptr, N + 2);
    int shared_variable = 0;

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, &shared_variable);

    consumers.resize(N);
    for (auto& consumer : consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, &shared_variable);
    }

    pthread_t interrupter;
    pthread_create(&interrupter, nullptr, consumer_interrupter_routine, nullptr);

    pthread_join(producer, nullptr);
    pthread_join(interrupter, nullptr);

    int result = 0;
    int* return_value;
    for (auto& consumer : consumers) {
        pthread_join(consumer, (void**) &return_value);
        result += *return_value;
        delete return_value;
    }

    return result;
}

int main(int argc, char** argv) {
    unsigned int threads_number = static_cast<unsigned int>(std::stoi(argv[1]));
    sleep_time = static_cast<unsigned int>(std::stoi(argv[2])) + 1;
    std::cout << run_threads(threads_number) << std::endl;
    return 0;
}