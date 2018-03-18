#include <pthread.h>
#include <iostream>
#include <stdio.h>

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

pthread_t producer_thread;
pthread_t consumer_thread;
pthread_t consumer_interruptor_thread;

bool isReady = false;

pthread_mutex_t producer_mutex;
pthread_mutex_t consumer_mutex;
pthread_cond_t producer_cond_t;
pthread_cond_t consumer_cond_t;

Value value;
int rest;

void* producer_routine(void* arg) {
    int nums_count;
    std::cout << "Enter numbers count: ";
    std::cin >> nums_count;
    rest = nums_count;

    if (rest == 0) {
        isReady = true;
        pthread_cond_signal(&producer_cond_t);
    }

    Value *value = (Value*)arg;
    int current;
    for (int i = 0; i < nums_count; i++) {
        std::cout << "[" << (i + 1) << "\\" << nums_count << "] number is : ";
        std::cin >> current;
        value->update(current);
        pthread_mutex_lock(&producer_mutex);
        isReady = true;
        pthread_cond_signal(&producer_cond_t);
        pthread_mutex_unlock(&producer_mutex);

        pthread_mutex_lock(&consumer_mutex);
        while (isReady) {
            pthread_cond_wait(&consumer_cond_t, &consumer_mutex);
        }
        pthread_mutex_unlock(&consumer_mutex);
    }
    pthread_cond_signal(&producer_cond_t);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    Value *value = (Value*)arg;
    int *sum = new int;
    *sum = 0;

    while(rest > 0){
        pthread_mutex_lock(&producer_mutex);
        while (!isReady) {
            pthread_cond_wait(&producer_cond_t, &producer_mutex);

        }
        pthread_mutex_unlock(&producer_mutex);

        *sum += value->get();
        rest--;

        pthread_mutex_lock(&consumer_mutex);
        isReady = false;
        pthread_cond_signal(&consumer_cond_t);
        pthread_mutex_unlock(&consumer_mutex);
    }
    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    while (rest != 0) {
        pthread_cancel(consumer_thread);
    }
}

int run_threads() {

    int *sum;
    rest = INT32_MAX;

    pthread_create(&producer_thread, NULL, producer_routine, &value);
    pthread_create(&consumer_thread, NULL, consumer_routine, &value);
    pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, (void**)&sum);


    int res = *sum;
    delete sum;
    return res;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
