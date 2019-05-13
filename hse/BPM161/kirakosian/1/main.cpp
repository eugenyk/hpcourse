#include <iostream>
#include <sstream>
#include <unistd.h>
#include <random>
#include <thread>

struct SharedData {
    int value;
    bool ready;
    bool finished;
};

struct InterruptorData {
    pthread_t* threads;
    int size;
};

pthread_barrier_t start_consumers_barrier;

pthread_mutex_t shared_data_mutex;
SharedData shared_data = { 0, false, false };

pthread_cond_t value_produced;
pthread_cond_t value_taken;

static thread_local int thread_sum;

int generate_random(const int& max) {
    static thread_local std::mt19937 generator(static_cast<unsigned long>(clock()));
    std::uniform_int_distribution<int> distribution(0, max);
    return distribution(generator);
}

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

    return nullptr;
}

void* consumer_routine(void* sleep_limit_ptr) {
    const int sleep_limit = *(int*) sleep_limit_ptr;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
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

        long sleep_time = generate_random(sleep_limit);
        if (sleep_limit != 0) {
            usleep(static_cast<__useconds_t>(sleep_time * 1000));
        }
    }
}

void* consumer_interruptor_routine(void* interruptor_data_ptr) {
    const InterruptorData interruptor_data = *(InterruptorData*) interruptor_data_ptr;

    pthread_barrier_wait(&start_consumers_barrier);

    // data race here is possible, but it wont be a mistake
    while (!shared_data.finished) {
        const int thread_index = generate_random(interruptor_data.size);
        pthread_cancel(interruptor_data.threads[thread_index]);
    }
    return nullptr;
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
    InterruptorData interruptor_data = { consumers, threads_number };
    pthread_create(&consumer_interruptor, nullptr, consumer_interruptor_routine, &interruptor_data);
    for (int i = 0; i < threads_number; i++) {
        pthread_create(&consumers[i], nullptr, consumer_routine, &sleep_limit);
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