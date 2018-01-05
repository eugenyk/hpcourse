#include <iostream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <sstream>
#include <cassert>

#include <pthread.h>

// LOGGING:
pthread_mutex_t cout_mutex = PTHREAD_MUTEX_INITIALIZER;
#define LOG(thread, x) do { \
        pthread_mutex_lock(&cout_mutex);\
        std::cout << "[" << #thread << "] " << x << std::endl; \
        pthread_mutex_unlock(&cout_mutex);\
    } while (0);

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

std::vector<int> read_numbers() {
    std::string s;
    std::getline(std::cin, s);
    std::istringstream cin(s);
    std::vector<int> res{std::istream_iterator<int>(cin), std::istream_iterator<int>()};
    return res;
}

pthread_t producer, consumer, interruptor;

bool consumer_started = false; // guarded by consumer_ready_mutex.
bool consumer_is_ready = false; // guarded by consumer_ready_mutex.
pthread_cond_t consumer_ready_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t consumer_ready_mutex = PTHREAD_MUTEX_INITIALIZER;

bool producer_is_ready = false; // guarded by producer_state_mutex.
pthread_mutex_t producer_state_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_state_condition = PTHREAD_COND_INITIALIZER;

pthread_mutex_t consumer_state_mutex = PTHREAD_MUTEX_INITIALIZER;

// TODO get rid of unused variables.
// TODO convert all global variables to local when it's possible.

enum struct ThreadState {NONE = 0, STARTED, RUNNING, WAITING, FINISHED} producer_state, consumer_state;

void set_state(ThreadState &state, ThreadState new_state, pthread_mutex_t &mutex, pthread_cond_t *condition = nullptr) {
    pthread_mutex_lock(&mutex);
    state = new_state;
    if (condition != nullptr) {
        pthread_cond_broadcast(condition);
    }
    pthread_mutex_unlock(&mutex);
}


//   Задача producer-потока - получить на вход список чисел,
// и по очереди использовать каждое значение из этого списка
// для обновления переменной разделяемой между потоками.
//   После этого поток должен дождаться реакции consumer-потока,
// и продолжить обновление значений только после того как
// поток-consumer принял это изменение.
//   Функция исполняющая код этого потока producer_routine
// должна принимать указатель на объект типа Value, и использовать его для обновления.
void* producer_routine(void* void_value) {
    Value *value = reinterpret_cast<Value *>(void_value);
    set_state(producer_state, ThreadState::STARTED, producer_state_mutex);

    // Wait for consumer to start
    LOG(producer, "Wait for consumer to start...");
    set_state(producer_state, ThreadState::WAITING, producer_state_mutex);

    pthread_mutex_lock(&consumer_ready_mutex);
    while (!consumer_started) { // if consumer is already started, no need to wait signal - it won't come.
        pthread_cond_wait(&consumer_ready_condition, &consumer_ready_mutex);
    }
    pthread_mutex_unlock(&consumer_ready_mutex);

    set_state(producer_state, ThreadState::RUNNING, producer_state_mutex);

    LOG(producer, "Reading data");
    auto numbers = read_numbers();

    // loop through each value
    for (auto num : numbers) {
        // update the value, notify consumer, wait for consumer to process
        LOG(producer, "Updating value...");
        value->update(num);
        LOG(producer, "Value updated with " + std::to_string(value->get()));

        // The same as with calling: "set_state(producer_state, ThreadState::WAITING, producer_state_mutex, &producer_state_condition);"
        // but with setting producer_is_ready = true;
        pthread_mutex_lock(&producer_state_mutex);
        producer_state = ThreadState::WAITING;
        producer_is_ready = true;
        pthread_cond_broadcast(&producer_state_condition);
        pthread_mutex_unlock(&producer_state_mutex);

        LOG(producer, "Waiting for news from the consumer...");
        pthread_mutex_lock(&consumer_ready_mutex);
        while (!consumer_is_ready) {
            pthread_cond_wait(&consumer_ready_condition, &consumer_ready_mutex);
            break;
        }
        consumer_is_ready = false;
        pthread_mutex_unlock(&consumer_ready_mutex);

        set_state(producer_state, ThreadState::RUNNING, producer_state_mutex);
    }

    set_state(producer_state, ThreadState::FINISHED, producer_state_mutex, &producer_state_condition);

    return NULL;
}

//   Задача consumer-потока отреагировать на каждое изменение переменной data
// и набирать сумму полученных значений.
//   После того как достигнуто последнее обновление,
// функция потока должна вернуть результирующую сумму.
//   Также этот поток должен защититься от попыток потока-interruptor его остановить.
//   Функция, исполняющая код этого потока, consumer_routine
// должна принимать указатель на тот же объект типа Value, и читать из него обновления.
void* consumer_routine(void* void_value) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    Value *value = static_cast<Value *>(void_value);

    set_state(consumer_state, ThreadState::STARTED, consumer_state_mutex);

    LOG(consumer, "Notifying about start...");
    // notify about start
    pthread_mutex_lock(&consumer_ready_mutex);
    consumer_started = true;
    pthread_cond_broadcast(&consumer_ready_condition);
    pthread_mutex_unlock(&consumer_ready_mutex);

    // allocate value for result
    Value *sum = new Value;

    set_state(consumer_state, ThreadState::RUNNING, consumer_state_mutex);

    while (true) {
        LOG(consumer, "Waiting for news from the producer...");
        pthread_mutex_lock(&producer_state_mutex);
        while (producer_state != ThreadState::FINISHED && !producer_is_ready) {
            pthread_cond_wait(&producer_state_condition, &producer_state_mutex);
        }

        if (producer_state == ThreadState::FINISHED) {
            pthread_mutex_unlock(&producer_state_mutex);
            break;
        }
        producer_is_ready = false;
        pthread_mutex_unlock(&producer_state_mutex);

        LOG(consumer, "Updating sum with value: " + std::to_string(value->get()));
        sum->update(sum->get() + value->get());

        // notifying producer about readiness to accept next value.
        pthread_mutex_lock(&consumer_ready_mutex);
        consumer_is_ready = true;
        pthread_cond_broadcast(&consumer_ready_condition);
        pthread_mutex_unlock(&consumer_ready_mutex);
    }
    set_state(consumer_state, ThreadState::FINISHED, consumer_state_mutex);

    // return pointer to result
    return sum;
}

//   Задача потока-interruptor проста:
// пока происходит процесс обновления значений,
// он должен постоянно пытаться остановить поток consumer.
//   Как только поток producer произвел последнее обновление, этот поток завершается.
void* consumer_interruptor_routine(void*) {
    // Wait for consumer to start
    LOG(interruptor, "Wait for consumer to start...");
    pthread_mutex_lock(&consumer_ready_mutex);
    while (!consumer_started) {
        pthread_cond_wait(&consumer_ready_condition, &consumer_ready_mutex);
    }
    pthread_mutex_unlock(&consumer_ready_mutex);

    LOG(interruptor, "Starting serving interruptions.");
    while (true) {
        pthread_mutex_lock(&producer_state_mutex);
        if (producer_state == ThreadState::FINISHED) {
            pthread_mutex_unlock(&producer_state_mutex);
            break;
        }
        pthread_mutex_unlock(&producer_state_mutex);
        pthread_cancel(consumer);
        LOG(interruptor, "Interrupting on consumer_ready_mutex");
        pthread_mutex_lock(&consumer_ready_mutex);
        pthread_mutex_unlock(&consumer_ready_mutex);
    }

    return NULL;
}

int run_threads() {
    // start 3 threads and wait until they're done
    Value *value = new Value;
    pthread_create(&producer, NULL, producer_routine, value);
    pthread_create(&consumer, NULL, consumer_routine, value);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);

    void *consumer_result_ptr, *interruptor_result_ptr;
    pthread_join(interruptor, &interruptor_result_ptr);
    pthread_join(consumer, &consumer_result_ptr);

    // return sum of update values seen by consumer
    Value *consumer_result_value_ptr = static_cast<Value *>(consumer_result_ptr);
    int consumer_result = consumer_result_value_ptr->get();
    delete consumer_result_value_ptr;
    delete value;
    return consumer_result;
}

int main() {
    FILE *f = freopen("readme.txt", "r", stdin);
    (void) f; // avoiding [-Wunused-result] warning.

    std::cout << run_threads() << std::endl;
    return 0;
}
