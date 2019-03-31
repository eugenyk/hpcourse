#include <iostream>
#include <sstream>

struct SharedData {
    int value;
    bool ready;
    bool finished;
};

pthread_barrier_t start_consumers_barrier;

pthread_mutex_t shared_data_mutex;
SharedData shared_data = { 0, false, false };

pthread_cond_t value_produced;
pthread_cond_t value_taken;

static thread_local int thread_sum;

void* producer_routine(void* input_stream_ptr) {
    // init input values
    const auto input_stream = (std::istream*) input_stream_ptr;
    std::string input_string;
    getline(*input_stream, input_string);
    std::istringstream string_stream(input_string);

    // wait for all threads
    pthread_barrier_wait(&start_consumers_barrier);

    int value;
    // reading values
    while (string_stream >> value) {
        pthread_mutex_lock(&shared_data_mutex);

        while (shared_data.ready) {
            pthread_cond_wait(&value_taken, &shared_data_mutex);
        }

        shared_data.value = value;
        shared_data.ready = true;

        pthread_cond_signal(&value_produced);
        pthread_mutex_unlock(&shared_data_mutex);
    }

    // send end signal
    pthread_mutex_lock(&shared_data_mutex);
    while (shared_data.ready) {
        pthread_cond_wait(&value_taken, &shared_data_mutex);
    }
    shared_data.finished = true;
    pthread_cond_broadcast(&value_produced);
    pthread_mutex_unlock(&shared_data_mutex);
}

void* consumer_routine(void* arg) {
    pthread_barrier_wait(&start_consumers_barrier);

    while (true) {
        pthread_mutex_lock(&shared_data_mutex);

        while (!shared_data.ready && !shared_data.finished) {
            pthread_cond_wait(&value_produced, &shared_data_mutex);
        }

        if (shared_data.finished) {
            pthread_mutex_unlock(&shared_data_mutex);
            return  &thread_sum;
        }
        thread_sum += shared_data.value;
        shared_data.ready = false;

        pthread_cond_signal(&value_taken);
        pthread_mutex_unlock(&shared_data_mutex);
    }
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&start_consumers_barrier);

    // wait for consumers to start

    // interrupt random consumer while producer is running
}

int run_threads(std::istream& input_stream, int threads_number, int sleep_limit) {
    if (threads_number <= 0 || sleep_limit < 0) {
        return 1;
    }
    pthread_barrier_init(&start_consumers_barrier, nullptr, static_cast<unsigned int>(threads_number) + 2);

    pthread_t producer;
    pthread_t consumer_interruptor;
    pthread_t consumers[threads_number];
    pthread_create(&producer, nullptr, producer_routine, &input_stream);
    pthread_create(&consumer_interruptor, nullptr, consumer_interruptor_routine, nullptr);
    for (int i = 0; i < threads_number; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, nullptr);
    }

    pthread_join(producer, nullptr);
    pthread_join(consumer_interruptor, nullptr);

    int sum = 0;
    for (int i = 0; i < threads_number; i++) {
        void* thread_result;
        pthread_join(consumers[i], &thread_result);
        sum += *(int*) thread_result;
    }

    std::cout << "Total: " << sum << std::endl;
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