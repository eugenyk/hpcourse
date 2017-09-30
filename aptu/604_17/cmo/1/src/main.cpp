#include <pthread.h>
#include <iostream>

class Value {
public:
    Value() : _value(0) {
    }

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};

static bool is_consumer_started = false;
static bool is_producer_finished = false;
static bool is_value_processed = true;

static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void* producer_routine(void* arg) {
    auto value = static_cast<Value*>(arg);

    // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    pthread_mutex_lock(&mutex);
    for (int x; std::cin >> x;) {
        while (!is_value_processed) {
            pthread_cond_wait(&cond, &mutex);
        }
        value->update(x);
        is_value_processed = false;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);

    is_producer_finished = true;
    return nullptr;
}

void* consumer_routine(void* arg) {
    auto value = static_cast<Value*>(arg);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    // notify about start
    pthread_mutex_lock(&mutex);
    is_consumer_started = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    // allocate value for result
    auto sum = new int;
    *sum = 0;

    // for every update issued by producer, read the value and add to sum
    pthread_mutex_lock(&mutex);
    while (!is_producer_finished) {
        while (is_value_processed) {
            pthread_cond_wait(&cond, &mutex);
        }
        *sum += value->get();
        is_value_processed = true;
        pthread_cond_signal(&cond);
    }
    pthread_mutex_unlock(&mutex);

    // return pointer to result
    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    auto consumer = static_cast<pthread_t*>(arg);

    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    // interrupt consumer while producer is running
    while (!is_producer_finished) {
        pthread_cancel(*consumer);
    }

    return nullptr;
}

int run_threads() {
    // start 3 threads and wait until they're done
    Value value;
    pthread_t producer, consumer, interruptor;
    pthread_create(&producer, nullptr, producer_routine, &value);
    pthread_create(&consumer, nullptr, consumer_routine, &value);
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumer);
    int* sum = nullptr;
    pthread_join(producer, nullptr);
    pthread_join(consumer, reinterpret_cast<void**>(&sum));
    pthread_join(interruptor, nullptr);

    // return sum of update values seen by consumer
    int result = *sum;
    delete sum;
    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
