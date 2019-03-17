#include <pthread.h>
#include <iostream>
#include <random>
#include <sstream>
#include <optional>
#include <chrono>
#include <thread>

using count_t = int;

enum class state_t {
    RUNNING,
    FINISHED
};

struct shared_primitives {
    pthread_mutex_t value_mutex;
    pthread_cond_t value_available;
    pthread_cond_t value_consumed;
    std::optional<int> value;

    state_t algorithm_state = state_t::RUNNING;
    pthread_rwlock_t state_lock;


    shared_primitives() {
        pthread_cond_init(&value_available, nullptr);
        pthread_cond_init(&value_consumed, nullptr);
        pthread_mutex_init(&value_mutex, nullptr);
        pthread_rwlock_init(&state_lock, nullptr);
    }
};

struct mutex_holder { // RAII
    explicit mutex_holder(pthread_mutex_t* mutex): mutex(mutex) {
        pthread_mutex_lock(mutex);
    }
    ~mutex_holder() {
        pthread_mutex_unlock(mutex);
    }

private:
    pthread_mutex_t *mutex;
};

struct rw_lock_holder { // RAII
    explicit rw_lock_holder(pthread_rwlock_t* lock, bool for_read = false): lock(lock) {
        if (for_read)
            pthread_rwlock_rdlock(lock);
        else
            pthread_rwlock_wrlock(lock);
    }
    ~rw_lock_holder() {
        pthread_rwlock_unlock(lock);
    }

private:
    pthread_rwlock_t *lock;
};

struct interruptor_data {
    std::vector<pthread_t>& consumers;
    shared_primitives& primitives;
};

static time_t consumer_sleep_time;
static pthread_barrier_t initialization_barrier;
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

        rw_lock_holder state_lock_holder(&primitives.state_lock);
        primitives.algorithm_state = state_t::FINISHED;

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

            while (!primitives.value.has_value()) {
                {
                    rw_lock_holder state_lock_holder(&primitives.state_lock, true);
                    if (primitives.algorithm_state == state_t::FINISHED)
                        return &partial_sum;
                }

                pthread_cond_wait(&primitives.value_available, &primitives.value_mutex);
            }

            partial_sum += primitives.value.value();
            primitives.value.reset();

            pthread_cond_signal(&primitives.value_consumed);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(random_millis()));
    }
}

void* consumer_interruptor_routine(void* arg) {
    interruptor_data &data = *static_cast<interruptor_data*>(arg);

    pthread_barrier_wait(&initialization_barrier);

    int consumers_number = static_cast<int>(data.consumers.size());

    while (true) {
        {
            rw_lock_holder state_lock_holder(&data.primitives.state_lock, true);
            if (data.primitives.algorithm_state == state_t::FINISHED)
                return nullptr;
        }
        pthread_cancel(data.consumers[rand() % consumers_number]);
    }
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

    interruptor_data data {consumers, primitives};

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, &data);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

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