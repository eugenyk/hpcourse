#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>

__thread int TLS_sum = 0;
unsigned N, max_time;

enum STATE {
    FINISHED, PRODUCED, CONSUMED
};

STATE current_state = CONSUMED;
std::vector<pthread_t> consumers;

pthread_mutex_t shared_variable_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t consumer_state_produced = PTHREAD_COND_INITIALIZER;
pthread_cond_t producer_state_taken = PTHREAD_COND_INITIALIZER;
pthread_barrier_t init_barrier;

void* producer_routine(void* arg) {
    // Wait for consumer to start
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    pthread_barrier_wait(&init_barrier);

    int* number = (int*)arg;
    std::string input;
    std::getline(std::cin, input);
    std::stringstream in(input);

    while (in >> *number) {
        pthread_mutex_lock(&shared_variable_mutex);
        current_state = PRODUCED;
        pthread_cond_signal(&consumer_state_produced);
        pthread_mutex_unlock(&shared_variable_mutex);

        pthread_mutex_lock(&shared_variable_mutex);
        while (current_state != CONSUMED) {
            pthread_cond_wait(&producer_state_taken, &shared_variable_mutex);
        }
        pthread_mutex_unlock(&shared_variable_mutex);
    }

    pthread_mutex_lock(&shared_variable_mutex);
    current_state = FINISHED;

    pthread_cond_broadcast(&consumer_state_produced);
    pthread_mutex_unlock(&shared_variable_mutex);
    return nullptr;
}


void* consumer_routine(void* arg) {
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&init_barrier);

    int* number = (int*)arg;

    while (true) {
        pthread_mutex_lock(&shared_variable_mutex);

        while (current_state == CONSUMED) {
            pthread_cond_wait(&consumer_state_produced, &shared_variable_mutex);
        }

        if (current_state == FINISHED) {
            pthread_mutex_unlock(&shared_variable_mutex);
            break;
        }

        TLS_sum += *number;
        current_state = CONSUMED;

        pthread_cond_signal(&producer_state_taken);
        pthread_mutex_unlock(&shared_variable_mutex);

        unsigned sleep_time = rand() % max_time;
        if (sleep_time) {
            timespec req = {0, 1000000 * sleep_time};
            nanosleep(&req, nullptr);
        }
    }

    return &TLS_sum;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start
    // interrupt random consumer while producer is running

    pthread_barrier_wait(&init_barrier);

    while (true) {
        pthread_mutex_lock(&shared_variable_mutex);

        if (current_state == FINISHED) {
            pthread_mutex_unlock(&shared_variable_mutex);
            break;
        }

        pthread_mutex_unlock(&shared_variable_mutex);

        pthread_cancel(consumers[rand() % consumers.size()]);

    }

    return nullptr;
}

void validate_status(int status) {
    if (status) {
        exit(status);
    }
}

int run_threads() {
    // start N threads and wait until they're done
    // return aggregated sum of values

    //setup init barrier for ensure all threads started
    pthread_barrier_init(&init_barrier, nullptr, N + 2);

    int shared_variable = 0, status = 0;
    //setup threads
    pthread_t producer, interruptor;
    status = pthread_create(&producer, nullptr, producer_routine, &shared_variable);
    validate_status(status);
    status = pthread_create(&interruptor, nullptr, consumer_interruptor_routine, nullptr);
    validate_status(status);

    for (size_t i = 0; i < N; i++) {
        status = pthread_create(&consumers[i], nullptr, consumer_routine, &shared_variable);
        validate_status(status);
    }

    status = pthread_join(interruptor, nullptr);
    validate_status(status);
    // calculate result
    int result = 0;
    for (size_t i = 0; i < N; i++) {
        void *ret_val;
        status = pthread_join(consumers[i], &ret_val);
        validate_status(status);
        result += *((int *) ret_val);
    }

    status = pthread_join(producer, nullptr);
    validate_status(status);

    pthread_mutex_destroy(&shared_variable_mutex);
    pthread_cond_destroy(&consumer_state_produced);
    pthread_cond_destroy(&producer_state_taken);
    pthread_barrier_destroy(&init_barrier);
    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Must have 2 passed arguments" << std::endl;
        exit(0);
    }

    N = (unsigned)std::stoi(argv[1]);
    consumers.resize(N);

    max_time = (unsigned)std::stoi(argv[2]) + 1u;
    std::cout << run_threads() << std::endl;

    return 0;
}
