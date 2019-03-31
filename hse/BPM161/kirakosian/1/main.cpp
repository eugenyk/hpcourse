#include <iostream>

struct SharedData {
    int value;
    bool ready;
};

pthread_barrier_t start_consumers_barrier;

pthread_mutex_t shared_data_mutex;
SharedData shared_data;

pthread_cond_t value_produced;
pthread_cond_t value_taken;

void* producer_routine(void* arg) {
    auto input_stream = (std::istream*) arg;
    pthread_barrier_wait(&start_consumers_barrier);

    int value;
    while (*input_stream >> value) {
        pthread_mutex_lock(&shared_data_mutex);

        while (shared_data.ready) {
            pthread_cond_wait(&value_taken, &shared_data_mutex);
        }

        shared_data.value = value;
        shared_data.ready = true;

        pthread_cond_signal(&value_produced);
        pthread_mutex_unlock(&shared_data_mutex);
    }
}

void* consumer_routine(void* arg) {
    pthread_barrier_wait(&start_consumers_barrier);

    int sum;
    while (true) {
        pthread_mutex_lock(&shared_data_mutex);

        while (!shared_data.ready) {
            pthread_cond_wait(&value_produced, &shared_data_mutex);
        }
        sum += shared_data.value;
        shared_data.ready = false;
        std::cout << "Thread " << pthread_self() << " has sum " << sum << std::endl;

        pthread_cond_signal(&value_taken);
        pthread_mutex_unlock(&shared_data_mutex);
    }
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&start_consumers_barrier);

    // wait for consumers to start

    // interrupt random consumer while producer is running
}

int run_threads(std::istream& input_stream, int threads_count, int sleep_limit) {
    if (threads_count <= 0 || sleep_limit < 0) {
        return 1;
    }
    pthread_barrier_init(&start_consumers_barrier, nullptr, static_cast<unsigned int>(threads_count) + 2);

    pthread_t producer;
    pthread_t consumer_interruptor;
    pthread_t consumers[threads_count];
    pthread_create(&producer, nullptr, producer_routine, &input_stream);
    pthread_create(&consumer_interruptor, nullptr, consumer_interruptor_routine, nullptr);
    for (int i = 0; i < threads_count; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, nullptr);
    }

    pthread_join(producer, nullptr);
    pthread_join(consumer_interruptor, nullptr);

    std::cout << "Run!";

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Invalid number of arguments" << std::endl;
    }

    int threads_number = std::stoi(argv[1]);
    int sleep_limit = std::stoi(argv[2]);
    if (run_threads(std::cin, threads_number, sleep_limit)) {
        std::cout << "Program did not finished successfully due errors" << std::endl;
        return 1;
    }
    std::cout << "Program finished successfully" << std::endl;

    return 0;
}