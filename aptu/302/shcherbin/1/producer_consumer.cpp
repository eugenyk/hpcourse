#include <iostream>
#include <pthread.h>
 
class Value {
public:
    Value() : _value(0), _has_value(false) { }
 
    void update(int value) {
        _value = value;
        _has_value = true;
    }

    void reset_has_value() {
        _has_value = false;
    }
 
    int get() const {
        return _value;
    }

    bool has_value() const {
        return _has_value;
    }

private:
    int _value;
    bool _has_value;
};

pthread_t producer_thread;
pthread_t consumer_thread;
pthread_t interruptor_thread;

pthread_mutex_t mutex;
pthread_cond_t cond;

bool is_initialized = false;
bool is_producer_finished = false;

void* producer_routine(void *arg) {
    // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!is_initialized) {
        pthread_cond_wait(&cond, &mutex);
    }

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    Value *value = (Value*) arg;
	int a;
    while (std::cin >> a) {
        value->update(a);
        pthread_cond_broadcast(&cond);
        while (value->has_value()) {
            pthread_cond_wait(&cond, &mutex);
        }
    }

    is_producer_finished = true;
    pthread_cond_broadcast(&cond);

    pthread_mutex_unlock(&mutex);

    return NULL;
}

void* consumer_routine(void *arg) {
    // notify about start
    // allocate value for result
    // for every update issued by producer_thread, read the value and add to sum
    // return pointer to result
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    Value *value = (Value*) arg;
    int *result = new int;
    *result = 0;

    pthread_mutex_lock(&mutex);
    is_initialized = true;
    while (true) {
        pthread_cond_broadcast(&cond);
        while (!value->has_value() && !is_producer_finished) {
            pthread_cond_wait(&cond, &mutex);
        }
        if (is_producer_finished) {
            break;
        }
        *result += value->get();
        value->reset_has_value();
    }
    pthread_mutex_unlock(&mutex);

    return result;
}

void* consumer_interruptor_routine(void* arg) {
    // wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!is_initialized) {
        pthread_cond_wait(&cond, &mutex);
    }

    // interrupt consumer while producer_thread is running                                          
	// the thread locks the mutex to check if the producer has finished
	while (!is_producer_finished) {
		pthread_mutex_unlock(&mutex);
		pthread_cancel(consumer_thread);
		pthread_mutex_lock(&mutex);
	}
	pthread_mutex_unlock(&mutex);

    (void) arg;
    return NULL;
}

int run_threads() {
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer

    Value value;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);

    pthread_create(&producer_thread, NULL, producer_routine, &value);
    pthread_create(&consumer_thread, NULL, consumer_routine, &value);
    pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, NULL);

    int *result_p;

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, (void**) &result_p);
	pthread_join(interruptor_thread, NULL);

    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);

    // not to leave result_p hanging
    int result = *result_p;
    delete result_p;

    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
