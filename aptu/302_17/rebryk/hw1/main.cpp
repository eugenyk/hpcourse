#include <pthread.h>
#include <iostream>

#define NOT_STARTED 0
#define OLD_VALUE 1
#define NEW_VALUE 2
#define FINISHED 3

volatile char state = NOT_STARTED;

pthread_mutex_t lock;
pthread_cond_t cond;
pthread_cond_t flag_cond;

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

void init() {
    pthread_mutex_init(&lock, NULL);
    pthread_cond_init(&cond, NULL);
    pthread_cond_init(&flag_cond, NULL);
}

void destroy() {
    pthread_mutex_destroy(&lock);
    pthread_cond_destroy(&cond);
    pthread_cond_destroy(&flag_cond);
}

void* producer_routine(void* arg) {
    pthread_mutex_lock(&lock);
    while (state == NOT_STARTED) {
        pthread_cond_wait(&flag_cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    int x;
    while (std::cin >> x) {
        pthread_mutex_lock(&lock);

        while (state == NEW_VALUE) {
            pthread_cond_wait(&cond, &lock);
        }

        state = NEW_VALUE;
        ((Value*) arg)->update(x);

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }

    pthread_mutex_lock(&lock);
    state = FINISHED;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&lock);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pthread_mutex_lock(&lock);
    state = OLD_VALUE;
    pthread_cond_broadcast(&flag_cond);
    pthread_mutex_unlock(&lock);

    int* sum = new int(0);

    while (1) {
        pthread_mutex_lock(&lock);

        while (state == OLD_VALUE) {
            pthread_cond_wait(&cond, &lock);
        }

        if (state == FINISHED) {
            break;
        }

        state = OLD_VALUE;
        *sum += ((Value*) arg)->get();

        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&lock);
    }

    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&lock);
    while (state == NOT_STARTED) {
        pthread_cond_wait(&flag_cond, &lock);
    }
    pthread_mutex_unlock(&lock);

    while (state != FINISHED) {
        pthread_mutex_lock(&lock);

        if (state != FINISHED) {
            pthread_cancel(*((pthread_t*) arg));
        }

        pthread_mutex_unlock(&lock);
    }
}

int run_threads() {
    init();

    pthread_t producer;
    pthread_t consumer;
    pthread_t interruptor;

    Value value;

    pthread_create(&producer, NULL, producer_routine, &value);
    pthread_create(&consumer, NULL, consumer_routine, &value);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumer);

    pthread_join(producer, NULL);
    pthread_join(interruptor, NULL);

    void* result;
    pthread_join(consumer, &result);

    destroy();

    return *((int*) result);
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}