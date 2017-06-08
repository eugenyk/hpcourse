#include <iostream>
#include <pthread.h>
#include <stdlib.h>

class Value {

private:
    int _value;
    bool _finished;
    pthread_mutex_t _mtx;
    bool _value_set;
    pthread_cond_t _value_cond;

public:
    Value() : _value(0), _finished(false), _value_set(false) {
        pthread_mutex_init(&_mtx, NULL);
        pthread_cond_init(&_value_cond, NULL);
    }

    void update(int value) {
        pthread_mutex_lock(&_mtx);
        while(_value_set == true) {
            pthread_cond_wait(&_value_cond, &_mtx);
        }
        _value = value;
        _value_set = true;
        pthread_cond_broadcast(&_value_cond);
        pthread_mutex_unlock(&_mtx);
    }

    int get(){
        pthread_mutex_lock(&_mtx);
        while(_value_set == false && _finished == false)  {
            pthread_cond_wait(&_value_cond, &_mtx);
        }
        int result = 0;
        // if the reason for the wake up is an actual value change
        if (_finished == false) {
            result = _value;
        }
        _value_set = false;
        pthread_cond_broadcast(&_value_cond);
        pthread_mutex_unlock(&_mtx);
        return result;
    }

    bool is_finished() {
        pthread_mutex_lock(&_mtx);
        bool result = _finished;
        pthread_mutex_unlock(&_mtx);
        return result;
    }

    void finish() {
        pthread_mutex_lock(&_mtx);
        _finished = true;
        pthread_cond_broadcast(&_value_cond);
        pthread_mutex_unlock(&_mtx);
    }
};

pthread_mutex_t mtx;
pthread_cond_t cond;
bool consumer_run = false;

void* producer_routine(void* arg) {
    Value* value = (Value*)arg;

  // Wait for consumer to start
    pthread_mutex_lock (&mtx);
    while (consumer_run == false) {
        pthread_cond_wait (&cond, &mtx);
    }
    pthread_mutex_unlock (&mtx);

  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    int val = 0;
    while (!std::cin.eof()) {
        std::cin >> val;
        value->update(val);
    }
    value->finish();

    return NULL;
}

void* consumer_routine(void* arg) {
    Value* value = (Value*)arg;
  // defend against the interruptor
    int s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (s != 0) {
        perror("pthread_setcancelstate");
    }

  // notify about start
    pthread_mutex_lock (&mtx);
    consumer_run = true;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock (&mtx);

  // allocate value for result
    int* result = (int*)malloc(sizeof(int));

  // for every update issued by producer, read the value and add to sum
    while(!value->is_finished()) {
        *result += value->get();
    }

  // tell the interruptor to stop interrupting us
    pthread_mutex_lock (&mtx);
    consumer_run = false;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock (&mtx);

  // return pointer to result
    return result;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_t* consumer_thread = (pthread_t*)arg;

  // wait for consumer to start
    pthread_mutex_lock (&mtx);
    while (consumer_run == false) {
        pthread_cond_wait (&cond, &mtx);
    }
    pthread_mutex_unlock (&mtx);

  // interrupt consumer while producer is running
    int s = 0;
    while(s == 0) {
        s = pthread_cancel(*consumer_thread);
    }
    return NULL;
}

int run_threads() {
    consumer_run = false;
    Value value;
    int* result;
    int ret_val;

  // start 3 threads and wait until they're done
    pthread_t producer_thread;
    pthread_t consumer_thread;
    pthread_t interruptor_thread;

    pthread_create(&producer_thread, NULL, producer_routine, &value);
    pthread_create(&consumer_thread, NULL, consumer_routine, &value);
    pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, (void**)&result);
    pthread_join(interruptor_thread, NULL);

  // return sum of update values seen by consumer
    ret_val = *result;
    free(result);
    return ret_val;
}

int main() {
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cond, NULL);
    std::cout << run_threads() << std::endl;
    return 0;
}
