#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

thread_local int sum = 0;
int wait_limit;
size_t consumer_count;

std::vector<pthread_t> consumers;

pthread_mutex_t read_mutex;
pthread_mutex_t write_mutex;
pthread_cond_t producer_condition;
pthread_cond_t consumer_condition;
pthread_barrier_t barrier;

int queued_number;
bool number_set = false;
bool running = true;

void* producer_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    std::string numbers;
    std::getline(std::cin, numbers);
    std::istringstream sin(numbers);

    int number;
    while (sin >> number) {
        pthread_mutex_lock(&write_mutex);
        while (number_set) {
            pthread_cond_wait(&producer_condition, &write_mutex);
        }
        queued_number = number;
        pthread_cond_signal(&consumer_condition);
        pthread_mutex_unlock(&write_mutex);
    }

    pthread_mutex_lock(&write_mutex);
    running = false;
    pthread_cond_broadcast(&consumer_condition);
    pthread_mutex_unlock(&write_mutex);

    return nullptr;
}

int gen_wait_time() {
    return rand() % (wait_limit + 1);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&barrier);

    while (running) {
        pthread_mutex_lock(&read_mutex);
        while (running && !number_set) {
            pthread_cond_wait(&consumer_condition, &read_mutex);
        }
        if (running) {
            pthread_mutex_lock(&write_mutex);
            sum += queued_number;
            number_set = false;
            pthread_cond_signal(&producer_condition);
            pthread_mutex_unlock(&write_mutex);
        }
        pthread_mutex_unlock(&read_mutex);

        std::this_thread::sleep_for(std::chrono::milliseconds(gen_wait_time()));
    }

    return reinterpret_cast<void *>(sum);
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&barrier);
    while (running) {
        pthread_cancel(consumers[rand() % consumer_count]);
    }
    return nullptr;
}

int run_threads() {
    pthread_barrier_init(&barrier, nullptr, static_cast<unsigned int>(consumer_count) + 3);

    pthread_t producer;
    pthread_create(&producer, nullptr, &producer_routine, nullptr);

    consumers.resize(consumer_count);
    for (pthread_t consumer : consumers) {
        pthread_create(&consumer, nullptr, &consumer_routine, nullptr);
    }

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, nullptr);

    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    int total = 0;
    for (pthread_t consumer : consumers) {
        int consumer_return_value;
        pthread_join(consumer, reinterpret_cast<void **>(&consumer_return_value));
        total += consumer_return_value;
    }

    pthread_mutex_destroy(&read_mutex);
    pthread_mutex_destroy(&write_mutex);
    pthread_cond_destroy(&producer_condition);
    pthread_cond_destroy(&consumer_condition);

    return total;
}

int main(int argc, char *argv[]) {
    consumer_count = std::stoul(argv[1]);
    wait_limit = std::stoi(argv[2]);


    std::cout << run_threads() << std::endl;
    return 0;
}