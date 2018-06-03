#include <pthread.h>
#include <iostream>

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
    volatile int _value;
};

pthread_mutex_t producer_mutex;
pthread_mutex_t consumer_mutex;

pthread_cond_t consumer_condition;
pthread_cond_t producer_condition;

pthread_t producer;
pthread_t consumer;
pthread_t interruptor;

volatile bool is_value_accepted = true;
volatile bool is_consumer_started = false;
volatile bool is_producer_active = true;


void* producer_routine(void* arg) {
    pthread_mutex_lock(&consumer_mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&consumer_condition, &consumer_mutex);
    }
    pthread_mutex_unlock(&consumer_mutex);

    Value* value = static_cast<Value *>(arg);
    int number;
    while (std::cin >> number) {
        pthread_mutex_lock(&producer_mutex);
        value->update(number);
        is_value_accepted = false;
        pthread_cond_broadcast(&producer_condition);
        pthread_mutex_unlock(&producer_mutex);

        pthread_mutex_lock(&consumer_mutex);
        while (!is_value_accepted) {
            pthread_cond_wait(&consumer_condition, &consumer_mutex);
        }

        pthread_mutex_unlock(&consumer_mutex);
    }

    pthread_mutex_lock(&producer_mutex);
    is_producer_active = false;
    pthread_cond_broadcast(&producer_condition);
    pthread_mutex_unlock(&producer_mutex);

    return NULL;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    Value* value = static_cast<Value *>(arg);
    int* sum = new int;
    *sum = 0;

    pthread_mutex_lock(&consumer_mutex);
    is_consumer_started = true;
    pthread_cond_broadcast(&consumer_condition);
    pthread_mutex_unlock(&consumer_mutex);

    while (true) {
        // waiting for producer with producer mutex
        pthread_mutex_lock(&producer_mutex);
        while (is_value_accepted && is_producer_active) {
            pthread_cond_wait(&producer_condition, &producer_mutex);
        }
        pthread_mutex_unlock(&producer_mutex);

        if (!is_producer_active) {
            break;
        }

        // consume value with consumer mutex
        pthread_mutex_lock(&consumer_mutex);
        *sum += value->get();
        is_value_accepted = true;
        pthread_cond_broadcast(&consumer_condition);
        pthread_mutex_unlock(&consumer_mutex);
    }

    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&consumer_mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&consumer_condition, &consumer_mutex);
    }
    pthread_mutex_unlock(&consumer_mutex);

    while (is_producer_active) {
        pthread_cancel(consumer);
    }

    return NULL;
}

int run_threads() {
    Value value;
    pthread_mutex_init(&consumer_mutex, NULL);
    pthread_mutex_init(&producer_mutex, NULL);

    pthread_cond_init(&producer_condition, NULL);
    pthread_cond_init(&consumer_condition, NULL);

    pthread_create(&producer, NULL, producer_routine, &value);
    pthread_create(&consumer, NULL, consumer_routine, &value);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);

    int* ptr_result = NULL;
    pthread_join(producer, NULL);
    pthread_join(consumer, reinterpret_cast<void **>(&ptr_result));
    pthread_join(interruptor, NULL);

    pthread_cond_destroy(&producer_condition);
    pthread_cond_destroy(&consumer_condition);
    pthread_mutex_destroy(&consumer_mutex);
    pthread_mutex_destroy(&producer_mutex);

    int result = *ptr_result;
    delete ptr_result;

    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}