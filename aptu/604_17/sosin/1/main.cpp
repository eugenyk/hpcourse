#include <pthread.h>
#include <assert.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <iterator>
#include <tuple>


#define NUM_THREADS 3


class Value {
public:
    Value() : _value(0) { }

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};


pthread_mutex_t mutex_data;
pthread_cond_t cond;
pthread_barrier_t barrier;


enum class Readiness {
    producer,
    consumer,
    none
};


enum class Signal {
    prod_completed,
    none
};


Readiness ready = Readiness::none;
Signal signal = Signal::none;


void* producer_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    auto* value = (Value*) arg;

    std::vector<int> values{std::istream_iterator<int>(std::cin),
                                std::istream_iterator<int>()};

    pthread_mutex_lock(&mutex_data);

    for (auto new_val: values) {
        value->update(new_val);
        ready = Readiness::producer;

        while (ready != Readiness::consumer) {
            pthread_cond_wait(&cond, &mutex_data);
        }
    }

    pthread_mutex_unlock(&mutex_data);

    signal = Signal::prod_completed;

    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    auto* value = (Value*) arg;

    auto* result = new int(0);

    while (signal != Signal::prod_completed) {

        if (ready == Readiness::producer) {
            pthread_mutex_lock(&mutex_data);

            *result += value->get();
            ready = Readiness::consumer;

            pthread_mutex_unlock(&mutex_data);
            pthread_cond_broadcast(&cond);
        }
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);

    pthread_exit(result);
}

void* consumer_interruptor_routine(void* arg) {
    pthread_barrier_wait(&barrier);

    auto thread = (pthread_t*) arg;

    while (signal != Signal::prod_completed) {
        pthread_cancel(*thread);
    }

    return nullptr;
}

int run_threads() {
    pthread_t prod_thread;
    pthread_t con_thread;
    pthread_t inter_thread;

    Value value;

    pthread_mutex_init(&mutex_data, nullptr);
    pthread_cond_init(&cond, nullptr);
    pthread_barrier_init(&barrier, nullptr, NUM_THREADS);

    assert(pthread_create(&prod_thread, nullptr,
                          producer_routine, &value) == 0);
    assert(pthread_create(&con_thread, nullptr,
                          consumer_routine, &value) == 0);
    assert(pthread_create(&inter_thread, nullptr,
                          consumer_interruptor_routine, &con_thread) == 0);

    void* result = nullptr;

    assert(pthread_join(prod_thread, nullptr) == 0);
    assert(pthread_join(con_thread, &result) == 0);
    assert(pthread_join(inter_thread, nullptr) == 0);

    pthread_mutex_destroy(&mutex_data);
    pthread_cond_destroy(&cond);
    pthread_barrier_destroy(&barrier);

    int sum = *(int*)result;
    delete (int*)result;

    return sum;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
