#include <pthread.h>
#include <iostream>
#include <vector>
#include <string>
#include <unistd.h>
#include <sstream>
#include <memory>

#ifdef DEBUG
#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#else
#define eprintf(...)
#endif

enum state {
    producer_is_working,
    consumer_is_working,
    finished
};

struct thread_sync {
    thread_sync() {
        pthread_cond_init(&update_cond, nullptr);
        pthread_cond_init(&process_cond, nullptr);
        pthread_mutex_init(&update_mutex, nullptr);
    }

    pthread_cond_t update_cond;
    pthread_mutex_t update_mutex;
    pthread_cond_t process_cond;
    state current_state = producer_is_working;
};

struct producer_arg {
    std::string line_with_numbers;
    int* current_value;

    producer_arg(std::string line) : current_value(new int(0)), line_with_numbers(line) {}

    ~producer_arg() {
        delete(current_value);
    }
};

thread_sync syncronization;
pthread_t producer;
pthread_t interrupter;
size_t millisec;
thread_local int partial_sum;
std::vector<pthread_t> threads;
pthread_barrier_t barrier;

void *producer_routine(void *arg) {
    eprintf("Producer: started\n");
    pthread_barrier_wait(&barrier);
    int* current_value = ((producer_arg*) arg)->current_value;
    std::string line_with_numbers = ((producer_arg*) arg)->line_with_numbers;
    std::stringstream ss;
    ss.str(line_with_numbers);
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    eprintf("Producer: started iterating\n");
    while (! ss.eof()) {
        int i;
        ss >> i;
        pthread_mutex_lock(&syncronization.update_mutex);
        *current_value = i;
        syncronization.current_state = consumer_is_working;
        eprintf("Producer: set value to %d\n", number);
        pthread_cond_signal(&syncronization.update_cond);
        while (syncronization.current_state == consumer_is_working) {
            pthread_cond_wait(&syncronization.process_cond, &syncronization.update_mutex);
        }
        pthread_mutex_unlock(&syncronization.update_mutex);
    }
    pthread_mutex_lock(&syncronization.update_mutex);
    syncronization.current_state = finished;
    pthread_cond_broadcast(&syncronization.update_cond);
    pthread_mutex_unlock(&syncronization.update_mutex);
    eprintf("Producer: finished\n");
    return nullptr;
}


void *consumer_routine(void *arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
#ifdef DEBUG
    int id = -1;
    for (int i = 0; i < threads.size(); i++) {
        if (pthread_equal(threads[i], pthread_self())) {
            id = i;
            break;
        }
    }
#endif
    eprintf("Consumer %d: started\n", id);
    int* val = (int *) arg;
    pthread_barrier_wait(&barrier);
    eprintf("Consumer %d: ready to loop \n", id);
    while (true) {
        pthread_mutex_lock(&syncronization.update_mutex);
        while (syncronization.current_state == producer_is_working) {
            pthread_cond_wait(&syncronization.update_cond, &syncronization.update_mutex);
        }
        if (syncronization.current_state == finished) {
            pthread_mutex_unlock(&syncronization.update_mutex);
            break;
        }
        partial_sum += *val;
        eprintf("Consumer %d: current sum = %d\n", id, partial_sum);
        syncronization.current_state = producer_is_working;
        pthread_cond_signal(&syncronization.process_cond);
        pthread_mutex_unlock(&syncronization.update_mutex);
        size_t time_to_sleep = millisec ? rand() % millisec : 0;
        if (time_to_sleep != 0) {
            usleep(time_to_sleep);
        }
    }
    eprintf("Consumer %d: final result=%d\n", id, partial_sum);
    return &partial_sum;
}


void *consumer_interruptor_routine(void *arg) {
    eprintf("Interruptor: started\n");
    pthread_barrier_wait(&barrier);
    eprintf("Interruptor: started looping\n");
    while(true) {
        size_t thread_id = rand() % threads.size();
        pthread_mutex_lock(&syncronization.update_mutex);
        bool is_finished = (syncronization.current_state == finished);
        pthread_mutex_unlock(&syncronization.update_mutex);
        if (is_finished) {
            break;
        } else {
            pthread_cancel(threads[thread_id]);
        }
    }
    eprintf("Interruptor: finished\n");
    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    eprintf("Starting run_threads()\n");
    std::string line_with_numbers;
    std::getline(std::cin, line_with_numbers);
    auto arg = std::make_unique<producer_arg>(std::move(line_with_numbers));
    pthread_create(&producer, nullptr, producer_routine, arg.get());
    for (int i = 0; i < threads.size(); i++) {
        pthread_create(&threads[i], nullptr, consumer_routine, arg->current_value);
    }
    pthread_create(&interrupter, nullptr, consumer_interruptor_routine, nullptr);
    // return aggregated sum of values
    int sum = 0;
    for (int i = 0; i < threads.size(); i++) {
        int* res = nullptr;
        pthread_join(threads[i], (void**) &res);
        sum += *res;
    }
    pthread_join(producer, nullptr);
    pthread_join(interrupter, nullptr);
    return sum;
}


int main(int argc, char **argv) {
    if (argc != 3) {
        std::cout << "Usage: first argument -- number of consumers, upper bound of time to sleep\n";
        return 0;
    }
    unsigned long number_of_consumers = std::stoul(argv[1]);
    millisec = std::stoul(argv[2]);
    threads.resize(number_of_consumers);
    pthread_barrier_init(&barrier, nullptr, static_cast<unsigned int>(number_of_consumers + 2));
    srand(static_cast<unsigned int>(time(NULL)));
    std::cout << run_threads() << std::endl;
    return 0;
}