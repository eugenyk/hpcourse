#include <pthread.h>
#include <iostream>
#include <random>
#include <sstream>
#include <optional>
#include <chrono>
#include <thread>

using count_t = unsigned int;

struct shared_primitives {
    pthread_mutex_t value_mutex;
    pthread_cond_t value_available;
    pthread_cond_t value_consumed;
    std::optional<int> value;

    shared_primitives() {
        pthread_cond_init(&value_available, nullptr);
        pthread_cond_init(&value_consumed, nullptr);
        pthread_mutex_init(&value_mutex, nullptr);
    }
};

struct mutex_holder { // RAI
    explicit mutex_holder(pthread_mutex_t* mutex): mutex(mutex) {
        pthread_mutex_lock(mutex);
    }
    ~mutex_holder() {
        pthread_mutex_unlock(mutex);
    }

private:
    pthread_mutex_t *mutex;
};

static time_t consumer_sleep_time;
static pthread_barrier_t initialization_barrier;
static volatile bool is_finished = false;
static __thread int partial_sum = 0;

void* producer_routine(void* arg) {
    shared_primitives &primitives = *static_cast<shared_primitives*>(arg);
    pthread_barrier_wait(&initialization_barrier);

    std::string input;
    getline(std::cin, input);

    std::istringstream in(input);

    int value;

    while (in >> value) {
        mutex_holder value_mutex_holder(&primitives.value_mutex);

        while (primitives.value.has_value())
            pthread_cond_wait(&primitives.value_consumed, &primitives.value_mutex);

        primitives.value = value;

        pthread_cond_signal(&primitives.value_available);
    }

    {
        mutex_holder value_mutex_holder(&primitives.value_mutex);

        while (primitives.value.has_value())
            pthread_cond_wait(&primitives.value_consumed, &primitives.value_mutex);

        is_finished = true;
        pthread_cond_broadcast(&primitives.value_available);
    }

    return nullptr;
}

time_t random_millis() {
    return rand() % (consumer_sleep_time + 1);
}

void* consumer_routine(void* arg) {
    shared_primitives &primitives = *static_cast<shared_primitives*>(arg);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&initialization_barrier);

    while (true) {
        {
            mutex_holder value_mutex_holder(&primitives.value_mutex);

            while (!is_finished && !primitives.value.has_value())
                pthread_cond_wait(&primitives.value_available, &primitives.value_mutex);

            if (is_finished)
                break;

            partial_sum += primitives.value.value();
            primitives.value.reset();

            pthread_cond_signal(&primitives.value_consumed);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(random_millis()));
    }

    return &partial_sum;
}

void* consumer_interruptor_routine(void* arg) {
    std::vector<pthread_t> &consumers = *static_cast<std::vector<pthread_t>*>(arg);

    pthread_barrier_wait(&initialization_barrier);

    while (!is_finished) {
        pthread_cancel(consumers[rand() % consumers.size()]);
    }

    return nullptr;
}

int run_threads(count_t consumers_number) {
    pthread_barrier_init(&initialization_barrier, nullptr, consumers_number + 2);
    shared_primitives primitives;

    pthread_t producer;
    pthread_create(&producer, nullptr, &producer_routine, &primitives);

    std::vector<pthread_t> consumers;

    for (size_t i = 0; i < (size_t)consumers_number; i++)
        consumers.emplace_back();

    for (size_t i = 0; i < (size_t)consumers_number; i++)
        pthread_create(&consumers[i], nullptr, &consumer_routine, &primitives);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, &consumers);

    pthread_join(producer, nullptr);

    int sum = 0;
    for (int i = 0; i < consumers_number; i++) {
        int* partial_sum;
        pthread_join(consumers[i], (void**)&partial_sum);
        sum += *partial_sum;
    }

    return sum;
}

int main(int argc, char** argv) {

    if (argc != 3) {
        std::cerr << "Usage: ./program [number of consumers] [wait limit in ms]\n";
    }

    count_t consumers_number = atoi(argv[1]);
    consumer_sleep_time = atoi(argv[2]);

    std::cout << run_threads(consumers_number) << std::endl;
    return 0;
}