#include <iostream>

pthread_barrier_t start_consumers_barrier;

void* producer_routine(void* arg) {
    // Wait for consumer to start

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
}

void* consumer_routine(void* arg) {
    std::cout << "Start thread " << pthread_self() << std::endl;
    pthread_barrier_wait(&start_consumers_barrier);
    // notify about start
    // for every update issued by producer, read the value and add to sum
    // return pointer to result (for particular consumer)
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumers to start

    // interrupt random consumer while producer is running
}

int run_threads(const std::istream& istream, int threads_count, int sleep_limit) {
    if (threads_count <= 0 || sleep_limit < 0) {
        return 1;
    }
    pthread_barrier_init(&start_consumers_barrier, nullptr, static_cast<unsigned int>(threads_count) + 1);

    pthread_t consumers[threads_count];
    for (int i = 0; i < threads_count; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, nullptr);
    }

    pthread_barrier_wait(&start_consumers_barrier);
    std::cout << "Run!";

    return 0;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cout << "Invalid number of arguments" << std::endl;
    }
    try {
        int threads_number = std::stoi(argv[1]);
        int sleep_limit = std::stoi(argv[2]);
        if (run_threads(std::cin, threads_number, sleep_limit)) {
            std::cout << "Illegal arguments";
        }
    } catch (const std::exception& e) {
        std::cout << "Illegal arguments";
    }
    return 0;
}