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

pthread_t producer_t, consumer_t, interruptor_t;
pthread_mutex_t mutex_val = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int status = 0;

void* producer_routine(void* arg) {
    Value * val = (Value *)arg;
    int x;
    pthread_mutex_lock(&mutex_val);
    while (std::cin >> x) {
        while (status == 0) {
            pthread_cond_wait(&cond, &mutex_val);
        }

        val->update(x);
        status = 0;
    }
    while (status == 0) {
        pthread_cond_wait(&cond, &mutex_val);
    }
    status = 2;
    pthread_mutex_unlock(&mutex_val);

    return NULL;
}

void* consumer_routine(void* arg) {
    int* result = new int;
    *result = 0;
    Value *val = (Value *) arg;
    int cur_st = 0;
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    while (cur_st < 2) {
        pthread_mutex_lock(&mutex_val);
        if (status == 0) {
            *result += val->get();
            status = 1;
            pthread_cond_broadcast(&cond);
        }
        cur_st = status;
        pthread_mutex_unlock(&mutex_val);
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    return result;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&mutex_val);
    while (status == 0) {
        pthread_cond_wait(&cond, &mutex_val);
    }
    pthread_mutex_unlock(&mutex_val);


    int cur_st = 0;
    while (cur_st != 2) {
        pthread_cancel(consumer_t);
        pthread_mutex_lock(&mutex_val);
        cur_st = status;
        pthread_mutex_unlock(&mutex_val);
    }

    return NULL;
}

int run_threads() {
    Value val;
    int* sum;
    pthread_create(&producer_t, NULL, producer_routine, (void *)&val);
    pthread_create(&consumer_t, NULL, consumer_routine, (void *)&val);
    pthread_create(&interruptor_t, NULL, consumer_interruptor_routine, NULL);

    pthread_join(producer_t, NULL);
    pthread_join(consumer_t, (void**)(&sum));
    pthread_join(interruptor_t, NULL);

    return *sum;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}