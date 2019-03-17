#include <pthread.h>
#include <iostream>
#include <sstream>
#include <unistd.h>

enum State {
    UPDATED, TAKEN, FINISHED
};

struct ProducerArguments {
    pthread_barrier_t *init_barrier;
    pthread_cond_t *update_condition;
    pthread_cond_t *take_condition;
    pthread_mutex_t *state_mutex;
    volatile State *current_state;
    volatile int *shared_value;
};

struct ConsumerArguments {
    pthread_barrier_t *init_barrier;
    pthread_cond_t *update_condition;
    pthread_cond_t *take_condition;
    pthread_mutex_t *state_mutex;
    volatile State *current_state;
    volatile int *shared_value;
    const int max_sleep_time;
    static thread_local int partial_result;
};

struct ConsumerInterruptorArguments {
    pthread_barrier_t *init_barrier;
    volatile State *current_state;
    const pthread_t *consumers;
    const unsigned int consumers_number;
};

thread_local int ConsumerArguments::partial_result;

void *producer_routine(void *arg) {
    auto *arguments = static_cast<ProducerArguments *>(arg);
    pthread_barrier_wait(arguments->init_barrier);
    std::string input;
    getline(std::cin, input);
    std::istringstream sin(input);
    std::string token;
    while (sin >> token) {
        *arguments->shared_value = static_cast<int>(strtol(token.c_str(), nullptr, 10));
        pthread_mutex_lock(arguments->state_mutex);
        *arguments->current_state = UPDATED;
        pthread_cond_signal(arguments->update_condition);
        while (*arguments->current_state != TAKEN) {
            pthread_cond_wait(arguments->take_condition, arguments->state_mutex);
        }
        pthread_mutex_unlock(arguments->state_mutex);
    }
    pthread_mutex_lock(arguments->state_mutex);
    *arguments->current_state = FINISHED;
    pthread_cond_broadcast(arguments->update_condition);
    pthread_mutex_unlock(arguments->state_mutex);
    return nullptr;
}

void *consumer_routine(void *arg) {
    auto *arguments = static_cast<ConsumerArguments *>(arg);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(arguments->init_barrier);
    arguments->partial_result = 0;
    while (true) {
        pthread_mutex_lock(arguments->state_mutex);
        while (*arguments->current_state == TAKEN) {
            pthread_cond_wait(arguments->update_condition, arguments->state_mutex);
        }
        if (*arguments->current_state == FINISHED) {
            pthread_mutex_unlock(arguments->state_mutex);
            break;
        }
        arguments->partial_result += *arguments->shared_value;
        *arguments->current_state = TAKEN;
        pthread_cond_signal(arguments->take_condition);
        pthread_mutex_unlock(arguments->state_mutex);
        sleep(rand() % (arguments->max_sleep_time + 1));
    }
    return &arguments->partial_result;
}

void *consumer_interruptor_routine(void *arg) {
    auto *arguments = static_cast<ConsumerInterruptorArguments *>(arg);
    pthread_barrier_wait(arguments->init_barrier);
    while (*arguments->current_state != FINISHED) {
        pthread_cancel(arguments->consumers[rand() % arguments->consumers_number]);
    }
    return nullptr;
}

int run_threads(unsigned int consumers_number, int max_sleep_time) {
    pthread_barrier_t init_barrier;
    pthread_barrier_init(&init_barrier, nullptr, consumers_number + 2);
    pthread_cond_t update_condition;
    pthread_cond_init(&update_condition, nullptr);
    pthread_cond_t take_condition;
    pthread_cond_init(&take_condition, nullptr);
    pthread_mutex_t state_mutex;
    pthread_mutex_init(&state_mutex, nullptr);
    volatile State current_state(TAKEN);
    volatile int shared_value = 0;
    pthread_t producer;
    pthread_t consumer_interruptor;
    pthread_t consumers[consumers_number];
    ProducerArguments producer_arguments = {
            &init_barrier,
            &update_condition,
            &take_condition,
            &state_mutex,
            &current_state,
            &shared_value
    };
    ConsumerArguments consumer_arguments = {
            &init_barrier,
            &update_condition,
            &take_condition,
            &state_mutex,
            &current_state,
            &shared_value,
            max_sleep_time
    };
    ConsumerInterruptorArguments consumer_interruptor_arguments = {
            &init_barrier,
            &current_state,
            consumers,
            consumers_number
    };
    pthread_create(&producer, nullptr, producer_routine, &producer_arguments);
    pthread_create(&consumer_interruptor, nullptr, consumer_interruptor_routine, &consumer_interruptor_arguments);
    for (unsigned int i = 0; i < consumers_number; ++i) {
        pthread_create(consumers + i, nullptr, consumer_routine, &consumer_arguments);
    }
    int result = 0;
    pthread_join(producer, nullptr);
    pthread_join(consumer_interruptor, nullptr);
    for (pthread_t thread : consumers) {
        void *partial_result;
        pthread_join(thread, (&partial_result));
        result += *static_cast<int *>(partial_result);
    }
    pthread_barrier_destroy(&init_barrier);
    pthread_cond_destroy(&update_condition);
    pthread_cond_destroy(&take_condition);
    pthread_mutex_destroy(&state_mutex);
    return result;
}

int main(int argc, char **argv) {
    auto consumers_number = static_cast<unsigned int>(strtol(argv[1], nullptr, 10));
    auto max_sleep_time = static_cast<int>(strtol(argv[2], nullptr, 10));
    std::cout << run_threads(consumers_number, max_sleep_time) << std::endl;
    return 0;
}
