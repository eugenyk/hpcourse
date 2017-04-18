#include <pthread.h>
#include <iostream>

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

pthread_mutex_t consumer_started_mutex;
pthread_cond_t consumer_started_cond;

pthread_mutex_t update_value_mutex;
pthread_cond_t update_value_cond;
pthread_cond_t updated_value_cond;

bool is_consumer_started = false;
bool is_value_updated = false;
bool is_producer_finished = false;

void* producer_routine(void* arg) {
    // Wait for consumer to start
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    Value* value = (Value*) arg;

    pthread_mutex_lock(&consumer_started_mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&consumer_started_cond, &consumer_started_mutex);
    }
    pthread_mutex_unlock(&consumer_started_mutex);

    int x;
    while (std::cin >> x) {
        pthread_mutex_lock(&update_value_mutex);
        value->update(x);
        is_value_updated = true;
        pthread_cond_signal(&update_value_cond);
        while (is_value_updated) {
            pthread_cond_wait(&updated_value_cond, &update_value_mutex);
        }
        pthread_mutex_unlock(&update_value_mutex);
    }

    pthread_mutex_lock(&update_value_mutex);
    is_producer_finished = true;
    pthread_cond_signal(&update_value_cond);
    pthread_mutex_unlock(&update_value_mutex);

    return 0;
}

void* consumer_routine(void* arg) {
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    Value* value = (Value*) arg;
    int* result_sum = new int(0);

    pthread_mutex_lock(&consumer_started_mutex);
    is_consumer_started = true;
    pthread_cond_broadcast(&consumer_started_cond);
    pthread_mutex_unlock(&consumer_started_mutex);

    while (!is_producer_finished) {
        pthread_mutex_lock(&update_value_mutex);
        while (!is_value_updated && !is_producer_finished) {
            pthread_cond_wait(&update_value_cond, &update_value_mutex);
        }
        if (is_producer_finished) {
            break;
        }
        *result_sum += value->get();
        is_value_updated = false;
        pthread_cond_signal(&updated_value_cond);
        pthread_mutex_unlock(&update_value_mutex);
    }

    return result_sum;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumer to start
    // interrupt consumer while producer is running

    pthread_t* thread_to_interrupt = (pthread_t*) arg;

    pthread_mutex_lock(&consumer_started_mutex);
    while (!is_consumer_started) {
        pthread_cond_wait(&consumer_started_cond, &consumer_started_mutex);
    }
    pthread_mutex_unlock(&consumer_started_mutex);

    while (!is_producer_finished) {
        pthread_cancel(*thread_to_interrupt);
    }

    return 0;
}

int run_threads() {
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer

    pthread_mutex_init(&consumer_started_mutex, NULL);
    pthread_cond_init(&consumer_started_cond, NULL);

    Value value;
    pthread_t producer_thread;
    pthread_t consumer_thread;
    pthread_t interruptor_thread;

    pthread_create(&producer_thread, NULL, producer_routine, &value);
    pthread_create(&consumer_thread, NULL, consumer_routine, &value);
    pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

    int* result_sum;

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, (void**) &result_sum);
    pthread_join(interruptor_thread, NULL);

    return *result_sum;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}