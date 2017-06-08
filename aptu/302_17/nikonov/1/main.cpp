#include <iostream>
#include <pthread.h>
#include <cstdlib>


class Value {
public:
    Value() : _value(0) {}

    void update(int value) {
        _value = value;
    }

    int get() const {
        return _value;
    }

private:
    int _value;
};

class BoxedValue {
public:
    BoxedValue() : _value(), _updated(false), _finished(false) {
        pthread_mutex_init(&_mutex, NULL);
        pthread_cond_init(&_cond, NULL);
    }

    void update(int value) {
        pthread_mutex_lock(&_mutex);
        while (_updated) {
            pthread_cond_wait(&_cond, &_mutex);
        }
        _updated = true;
        _value.update(value);
        pthread_cond_broadcast(&_cond);
        pthread_mutex_unlock(&_mutex);
    }

    int get() {
        pthread_mutex_lock(&_mutex);
        while (!_updated && !_finished) {
            pthread_cond_wait(&_cond, &_mutex);
        }
        int res = 0;
        if (_updated) {
            res = _value.get();
        }
        _updated = false;
        pthread_cond_broadcast(&_cond);
        pthread_mutex_unlock(&_mutex);
        return res;
    }

    void finish() {
        pthread_mutex_lock(&_mutex);
        _finished = true;
        pthread_cond_broadcast(&_cond);
        pthread_mutex_unlock(&_mutex);
    }

    bool is_alive() {
        pthread_mutex_lock(&_mutex);
        bool res = !_finished || _updated;
        pthread_mutex_unlock(&_mutex);
        return res;
    }

private:
    Value _value;
    bool _updated;
    bool _finished;
    pthread_mutex_t _mutex;
    pthread_cond_t _cond;

};

pthread_mutex_t mutex;
pthread_cond_t cond;
bool is_consumer_working = false;

void wait_for_consumer() {
    pthread_mutex_lock(&mutex);
    while (!is_consumer_working) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

bool is_consumer_alive() {
    pthread_mutex_lock(&mutex);
    bool res = is_consumer_working;
    pthread_mutex_unlock(&mutex);
    return res;
}

void *producer_routine(void *boxed_value) {
    BoxedValue *value = static_cast<BoxedValue*>(boxed_value);

    // Wait for consumer to start
    wait_for_consumer();

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    int next_val;
    while (std::cin >> next_val) {
        value->update(next_val);
    }
    value->finish();
    return 0;
}

void *consumer_routine(void *boxed_value) {
    BoxedValue *value = static_cast<BoxedValue*>(boxed_value);
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    // notify about start
    pthread_mutex_lock(&mutex);
    is_consumer_working = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);

    // allocate value for result
    int *res = new int(0);
    
    // for every update issued by producer, read the value and add to sum
    while (value->is_alive()) {
        *res += value->get();
    }

    pthread_mutex_lock(&mutex);
    is_consumer_working = false;
    pthread_mutex_unlock(&mutex);

    // return pointer to result
    return res;
}

void *consumer_interruptor_routine(void *consumer_thread) {
    pthread_t *thread = static_cast<pthread_t*>(consumer_thread);

    // wait for consumer to start
    wait_for_consumer();

    // interrupt consumer while producer is running
    while(is_consumer_alive()) {
        pthread_cancel(*thread);
    }
    return 0;
}

int run_threads() {
    BoxedValue *b_val = new BoxedValue();
    int *consumer_result;

    // start 3 threads and wait until they're done
    is_consumer_working = false;
    pthread_t producer;
    pthread_t consumer;
    pthread_t interruptor;
    pthread_create(&producer, NULL, producer_routine, b_val);
    pthread_create(&consumer, NULL, consumer_routine, b_val);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumer);

    pthread_join(producer, NULL);
    pthread_join(consumer, reinterpret_cast<void**>(&consumer_result));
    pthread_join(interruptor, NULL);

    // return sum of update values seen by consumer
    int res = *consumer_result;
    delete(b_val);
    delete(consumer_result);
    return res;
}

int main() {
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    std::cout << run_threads() << std::endl;
    return 0;
}