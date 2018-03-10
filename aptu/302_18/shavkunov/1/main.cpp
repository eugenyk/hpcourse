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
    int _value;
};

pthread_mutex_t mutex;
pthread_cond_t condition;

pthread_t producer;
pthread_t consumer;
pthread_t interruptor;

bool is_value_accepted = true;
bool is_consumer_started = false;
bool is_producer_active = false;


void wait_for_consumer() {
    pthread_mutex_lock(&mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&condition, &mutex);
    }
}

void* producer_routine(void* arg) {
    is_producer_active = true;
    Value* value = static_cast<Value *>(arg);

    wait_for_consumer();

    int number;
    while (std::cin >> number) {
        value->update(number);
        pthread_cond_broadcast(&condition);
        is_value_accepted = false;

        while (!is_value_accepted) {
            pthread_cond_wait(&condition, &mutex);
        }
    }

    is_producer_active = false;
    pthread_cond_broadcast(&condition);

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    Value* value = static_cast<Value *>(arg);
    int* sum = new int;
    *sum = 0;

    pthread_mutex_lock(&mutex);
    is_consumer_started = true;
    pthread_cond_broadcast(&condition); // notify producer about starting

    while (true) {
        while(is_value_accepted && is_producer_active) {
            pthread_cond_wait(&condition, &mutex); // waiting for new value
        }

        if (!is_producer_active) {
            break;
        }

        *sum += value->get();
        is_value_accepted = true;
        pthread_cond_broadcast(&condition);
    }

    pthread_mutex_unlock(&mutex);

    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    wait_for_consumer();

    while (is_producer_active) {
        pthread_mutex_unlock(&mutex);
        pthread_cancel(consumer);
        pthread_mutex_lock(&mutex);
    }

    pthread_mutex_unlock(&mutex);
    return NULL;
}

int run_threads() {
    Value value;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);

    pthread_create(&producer, NULL, producer_routine, &value);
    pthread_create(&consumer, NULL, consumer_routine, &value);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);

    int* ptr_result = NULL;
    pthread_join(producer, NULL);
    pthread_join(consumer, reinterpret_cast<void **>(&ptr_result));
    pthread_join(interruptor, NULL);

    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);

    int result = *ptr_result;
    delete ptr_result;

    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}