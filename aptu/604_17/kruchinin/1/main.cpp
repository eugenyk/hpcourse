#include <iostream>
#include <vector>
#include <pthread.h>


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

static pthread_t producer, consumer, interruptor;
static pthread_cond_t cond_empty, cond_consumer;
static pthread_mutex_t mutex_empty, mutex_start;
static bool consumer_started = false;
static bool is_empty = true;
static bool producer_finished = false;

void* producer_routine(void* arg) {
    Value* value = static_cast<Value *>(arg);

    pthread_mutex_lock(&mutex_start);
    while (!consumer_started) {
        pthread_cond_wait(&cond_consumer, &mutex_start);
    }
    pthread_mutex_unlock(&mutex_start);

    std::vector<int> data;
    {
        int number;
        while (std::cin >> number) {
            data.push_back(number);
        }
    }

    for (uint32_t i = 0; i < data.size(); i++) {
        pthread_mutex_lock(&mutex_empty);
        value->update(data[i]);

        is_empty = false;
        pthread_cond_signal(&cond_empty);

        while (!is_empty) {
            pthread_cond_wait(&cond_empty, &mutex_empty);
        }
        pthread_mutex_unlock(&mutex_empty);
    }

    pthread_mutex_lock(&mutex_empty);
    producer_finished = true;
    pthread_cond_signal(&cond_empty);
    pthread_mutex_unlock(&mutex_empty);

    return nullptr;
}

void* consumer_routine(void* arg) {
    Value* value = static_cast<Value *>(arg);

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_mutex_lock(&mutex_start);
    consumer_started = true;
    pthread_cond_broadcast(&cond_consumer);
    pthread_mutex_unlock(&mutex_start);

    int* result = new int();

    while (true) {
        pthread_mutex_lock(&mutex_empty);

        while (is_empty && !producer_finished) {
            pthread_cond_wait(&cond_empty, &mutex_empty);
        }

        if (producer_finished) {
            pthread_mutex_unlock(&mutex_empty);
            break;
        }

        *result += value->get();
        is_empty = true;

        pthread_cond_signal(&cond_empty);
        pthread_mutex_unlock(&mutex_empty);
    }

    return result;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&mutex_start);
    while (!consumer_started) {
        pthread_cond_wait(&cond_consumer, &mutex_start);
    }
    pthread_mutex_unlock(&mutex_start);

    while (!producer_finished) {
        pthread_cancel(consumer);
    }

    return nullptr;
}

int run_threads() {
    Value* value = new Value();

    pthread_create(&producer, nullptr, producer_routine, value);
    pthread_create(&consumer, nullptr, consumer_routine, value);
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, value);

    void* result_join;
    pthread_join(producer, nullptr);
    pthread_join(consumer, &result_join);
    pthread_join(interruptor, nullptr);

    int result = *static_cast<int*>(result_join);
    delete static_cast<int*>(result_join);
    delete value;

    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
