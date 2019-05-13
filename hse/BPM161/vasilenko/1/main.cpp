#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <thread>

thread_local int sum = 0;
int wait_limit;
size_t consumer_count;

std::vector<pthread_t> consumers;

pthread_mutex_t number_mutex;
pthread_cond_t producer_condition;
pthread_cond_t consumer_condition;
pthread_barrier_t barrier;

bool number_set = false;
bool running = true;

void* producer_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    std::string numbers;
    std::getline(std::cin, numbers);
    std::istringstream sin(numbers);

    int number;
    while (sin >> number) {
        pthread_mutex_lock(&number_mutex);

        int *queued_number = reinterpret_cast<int*>(arg);
        *queued_number = number;
        number_set = true;
        pthread_cond_signal(&consumer_condition);

        while (number_set) {
            pthread_cond_wait(&producer_condition, &number_mutex);
        }
        pthread_mutex_unlock(&number_mutex);
    }

    pthread_mutex_lock(&number_mutex);
    running = false;
    pthread_cond_broadcast(&consumer_condition);
    pthread_mutex_unlock(&number_mutex);

    return nullptr;
}

int gen_wait_time() {
    return rand() % (wait_limit + 1);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&barrier);

    pthread_mutex_lock(&number_mutex);
    while (running) {
        while (running && !number_set) {
            pthread_cond_wait(&consumer_condition, &number_mutex);
        }
        if (running) {
            int queued_number = *reinterpret_cast<int*>(arg);
            sum += queued_number;
            number_set = false;
            pthread_cond_signal(&producer_condition);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(gen_wait_time()));
    }
    pthread_mutex_unlock(&number_mutex);

    auto res = new int(sum);
    return res;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    pthread_mutex_lock(&number_mutex);
    while (running) {
        pthread_mutex_unlock(&number_mutex);
        pthread_cancel(consumers[rand() % consumer_count]);
        pthread_mutex_lock(&number_mutex);
    }
    pthread_mutex_unlock(&number_mutex);

    return nullptr;
}

int run_threads() {
    int queued_number;
    pthread_barrier_init(&barrier, nullptr, static_cast<unsigned int>(consumer_count) + 3);

    pthread_t producer;
    pthread_create(&producer, nullptr, &producer_routine, &queued_number);

    consumers.resize(consumer_count);

    for (pthread_t& consumer : consumers) {
        pthread_create(&consumer, nullptr, &consumer_routine, &queued_number);
    }

    pthread_t interrupter;
    pthread_create(&interrupter, nullptr, &consumer_interruptor_routine, nullptr);

    pthread_barrier_wait(&barrier);
    pthread_barrier_destroy(&barrier);

    pthread_join(producer, nullptr);
    pthread_join(interrupter, nullptr);

    int total = 0;
    for (pthread_t& consumer : consumers) {
        int *consumer_return_value;
        pthread_join(consumer, reinterpret_cast<void **>(&consumer_return_value));
        total += *consumer_return_value;
        delete consumer_return_value;
    }

    pthread_mutex_destroy(&number_mutex);
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