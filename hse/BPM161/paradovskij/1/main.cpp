#include <pthread.h>
#include <iostream>
#include <vector>

#include <random>
#include <functional>

#include <sstream>

enum class state {
    PRODUCING,
    AWAITING_CONSUME,
    CONSUMING,
    AWAITING_PRODUCE,
    STOPPED
};

unsigned int number_of_consumer_threads;
unsigned int time_to_sleep;

pthread_barrier_t start_barrier;
std::vector<pthread_t> consumers;

struct shared_things {
    pthread_mutex_t shared_things_mutex = PTHREAD_MUTEX_INITIALIZER;
    state system_state = state::AWAITING_PRODUCE;

    unsigned int update_variable = 0;

    pthread_cond_t produce_finished_cond = PTHREAD_COND_INITIALIZER;
    pthread_cond_t consume_finished_cond = PTHREAD_COND_INITIALIZER;
};


void* producer_routine(void* arg) {
    auto* things = static_cast<shared_things *>(arg);

    // Wait for consumer to start
    pthread_barrier_wait(&start_barrier);

    std::string input;
    std::getline(std::cin, input);
    std::istringstream stream(input);

    while (!stream.eof()) {
        unsigned int current;
        stream >> current;

        // save, notify
        pthread_mutex_lock(&things->shared_things_mutex);
        things->system_state = state::PRODUCING;
        things->update_variable = current;

        things->system_state = state::AWAITING_CONSUME;
        pthread_cond_signal(&things->produce_finished_cond);

        // wait until consumed
        while (things->system_state != state::AWAITING_PRODUCE) {
            pthread_cond_wait(&things->consume_finished_cond, &things->shared_things_mutex);
        }
        pthread_mutex_unlock(&things->shared_things_mutex);
    }

    pthread_mutex_lock(&things->shared_things_mutex);
    things->system_state = state::STOPPED;
    pthread_cond_broadcast(&things->produce_finished_cond);
    pthread_mutex_unlock(&things->shared_things_mutex);

    return nullptr;
}

__thread unsigned int single_consumer_sum = 0;
void* consumer_routine(void* arg) {
    auto* things = static_cast<shared_things *>(arg);

    auto generator = std::mt19937(std::random_device()());
    auto distributor = std::uniform_int_distribution<int>(0, time_to_sleep);
    auto local_random = std::bind(distributor, generator);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_barrier_wait(&start_barrier);

    while (true) {
        pthread_mutex_lock(&things->shared_things_mutex);
        while (things->system_state != state::AWAITING_CONSUME && things->system_state != state::STOPPED) {
            pthread_cond_wait(&things->produce_finished_cond, &things->shared_things_mutex);
        }
        if (things->system_state == state::STOPPED) {
            pthread_mutex_unlock(&things->shared_things_mutex);
            break;
        }

        things->system_state = state::CONSUMING;
        int current = things->update_variable;
        single_consumer_sum += current;

        things->system_state = state::AWAITING_PRODUCE;
        pthread_cond_signal(&things->consume_finished_cond);
        pthread_mutex_unlock(&things->shared_things_mutex);

        // i sleep
        long time_nano = local_random() * 1000000l;
        long billion = 1000000000;
        timespec sleep_time{ time_nano / billion, time_nano % billion };
        nanosleep(&sleep_time, nullptr);
    }

    return &single_consumer_sum;
}

void* consumer_interruptor_routine(void* arg) {
    auto* things = static_cast<shared_things *>(arg);
    pthread_barrier_wait(&start_barrier);

    auto generator = std::mt19937(std::random_device()());
    auto distributor = std::uniform_int_distribution<int>(0, number_of_consumer_threads - 1);
    auto random = std::bind(distributor, generator);

    while (things->system_state != state::STOPPED) {
        pthread_cancel(consumers[random()]);
    }

    return nullptr;
}

int run_threads() {
    shared_things things;
    pthread_barrier_init(&start_barrier, nullptr, number_of_consumer_threads + 2);

    consumers.resize(number_of_consumer_threads);
    for (int i = 0; i < number_of_consumer_threads; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, &things);
    }

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, &things);
    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &things);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    unsigned int result = 0;
    for (int i = 0; i < number_of_consumer_threads; i++) {
        int* res;
        pthread_join(consumers[i], (void **) &res);
        result += *res;
    }

    // destruction
    pthread_barrier_destroy(&start_barrier);
    pthread_mutex_destroy(&things.shared_things_mutex);
    pthread_cond_destroy(&things.produce_finished_cond);
    pthread_cond_destroy(&things.consume_finished_cond);

    return result;
}

int main(int argc, char** argv) {
    number_of_consumer_threads = static_cast<unsigned int>(std::stoi(argv[1]));
    time_to_sleep = static_cast<unsigned int>(std::stoi(argv[2]));

    std::cout << run_threads() << std::endl;
    return 0;
}