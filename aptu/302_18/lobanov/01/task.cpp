#include <pthread.h>
#include <iostream>


using namespace std;


enum Status {
    CONSUMER_NOT_STARTED, ELEMENT_REQUASTED, ELEMENT_WAITING, END_OF_DATA
};

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

pthread_mutex_t status_mutex;
pthread_cond_t status_changed_condition;
volatile Status status = CONSUMER_NOT_STARTED;

// mutex should be locked
void wait_status(Status expectation) {
    while (status != expectation) {
        pthread_cond_wait(&status_changed_condition, &status_mutex);
    }
}

// mutex should be locked
void skip_status(Status needless) {
    while (status == needless) {
        pthread_cond_wait(&status_changed_condition, &status_mutex);
    }
}

void* producer_routine(void* arg) {
    Value* value = (Value*) arg;
    int next_value;
    while (cin >> next_value) {
        pthread_mutex_lock(&status_mutex);               // lock
        wait_status(ELEMENT_REQUASTED);
        value->update(next_value);
        status = ELEMENT_WAITING;
        pthread_cond_broadcast(&status_changed_condition);
        pthread_mutex_unlock(&status_mutex);            // unlock
    }
    pthread_mutex_lock(&status_mutex);                  // lock
    wait_status(ELEMENT_REQUASTED);
    status = END_OF_DATA;
    pthread_cond_broadcast(&status_changed_condition);
    pthread_mutex_unlock(&status_mutex);                // unlock
    return (void*) 0;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    Value* value = (Value*) arg;
    Value* sum = new Value();
    pthread_mutex_lock(&status_mutex);               // lock
    status = ELEMENT_REQUASTED;
    pthread_cond_broadcast(&status_changed_condition);
    pthread_mutex_unlock(&status_mutex);             // unlock
    while (true) {
        pthread_mutex_lock(&status_mutex);               // lock
        skip_status(ELEMENT_REQUASTED);
        if (status == END_OF_DATA) {
            pthread_mutex_unlock(&status_mutex);         // unlock
            break;
        }
        sum->update(sum->get() + value->get());
        status = ELEMENT_REQUASTED;
        pthread_cond_broadcast(&status_changed_condition);
        pthread_mutex_unlock(&status_mutex);             // unlock
    }
    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_t consumer_thread = *(pthread_t*)arg;

    pthread_mutex_lock(&status_mutex);
    skip_status(CONSUMER_NOT_STARTED);
    pthread_mutex_unlock(&status_mutex);
    while(status != END_OF_DATA) {
        pthread_cancel(consumer_thread);
    }
    return (void*) 0;
}

int run_threads() {
    Value* value = new Value();
    void* holder;
    pthread_t consumer;
    pthread_t producer;
    pthread_t consumer_interruptor;
    pthread_create(&producer, NULL, &producer_routine, value);
    pthread_create(&consumer, NULL, &consumer_routine, value);
    pthread_create(&consumer_interruptor, NULL, &consumer_interruptor_routine, &consumer);
    pthread_join(producer, NULL);
    pthread_join(consumer, &holder);
    pthread_join(consumer_interruptor, NULL);
    Value* result = (Value*) holder;
    delete(value);
    return result->get();
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
