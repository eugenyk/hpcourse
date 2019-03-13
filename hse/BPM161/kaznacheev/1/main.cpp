#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include <vector>
#include <unistd.h>

struct interruptor_data {
    pthread_t *threads;
    int size;
};

int shared_value;
pthread_cond_t can_read;
pthread_cond_t can_write;
pthread_mutex_t shared_value_mutex;
pthread_barrier_t consumer_barrier;

bool is_ready = false;
bool is_over = false;

__thread int partial_sum;

void* producer_routine(void* arg) {
    pthread_barrier_wait(&consumer_barrier);

    int value;

    while (std::cin >> value) {
        pthread_mutex_lock(&shared_value_mutex);

        while (is_ready) {
            pthread_cond_wait(&can_write, &shared_value_mutex);
        }

        shared_value = value;
        is_ready = true;

        pthread_cond_signal(&can_read);

        pthread_mutex_unlock(&shared_value_mutex);
    }

    pthread_mutex_lock(&shared_value_mutex);
    while (is_ready) {
        pthread_cond_wait(&can_write, &shared_value_mutex);
    }
    is_over = true;
    pthread_cond_broadcast(&can_read);
    pthread_mutex_unlock(&shared_value_mutex);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    int sleep_time = *((int*) arg);

    pthread_barrier_wait(&consumer_barrier);

    while (true) {
        pthread_mutex_lock(&shared_value_mutex);

        while (!is_ready && !is_over) {
            pthread_cond_wait(&can_read, &shared_value_mutex);
        }

        if (is_ready) {
            partial_sum += shared_value;
            is_ready = false;
            pthread_cond_signal(&can_write);

            pthread_mutex_unlock(&shared_value_mutex);

            if (sleep_time > 0) {
                usleep((__useconds_t) abs(rand()) % (sleep_time * 1000));
            }
        }

        if (is_over) {
            pthread_mutex_unlock(&shared_value_mutex);
            return (void*) partial_sum;
        }
    }
}

void* consumer_interruptor_routine(void* arg) {
    auto data = (interruptor_data*) arg;
    pthread_t* threads = data->threads;
    int size = data->size;

    pthread_barrier_wait(&consumer_barrier);

    while (!is_over) {
        int thread_index = abs(rand()) % size;
        pthread_cancel(threads[thread_index]);
    }
    return nullptr;
}

int run_threads(int thread_count, int sleep_time) {
    pthread_barrier_init(&consumer_barrier, nullptr, (unsigned int)thread_count + 2);

    pthread_t threads[thread_count];

    pthread_t producer_thread;

    if (pthread_create(&producer_thread, nullptr, producer_routine, nullptr)) {
        std::cerr << "Error creating producer thread" << std::endl;
        return 0;
    }

    pthread_t interruptor_thread;
    interruptor_data data{threads, thread_count};
    if (pthread_create(&interruptor_thread, nullptr, consumer_interruptor_routine, &data)) {
        std::cerr << "Error creating interruptor thread" << std::endl;
        return 0;
    }
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(threads + i, nullptr, consumer_routine, &sleep_time)) {
            std::cerr << "Error creating consumer thread #" << i << std::endl;
            return 0;
        }
    }

    pthread_join(producer_thread, nullptr);
    pthread_join(interruptor_thread, nullptr);

    int result = 0;
    int consumer_result;
    for (int i = 0; i < thread_count; i++) {
        if(pthread_join(threads[i], (void**)&consumer_result)) {
            std::cerr << "Error joining consumer thread #" << i << std::endl;
            return 0;
        }
        result += consumer_result;
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Invalid argument count" << std::endl;
    }
    try {
        int thread_count = std::stoi(argv[1]);
        int sleep_time = std::stoi(argv[2]);
        std::cout << run_threads(thread_count, sleep_time) << std::endl;
    } catch (std::exception &e) {
        std::cerr << "Invalid argument values" << std::endl;
    }
    return 0;
}
