#include <pthread.h>
#include <iostream>

pthread_mutex_t produce_mutex;
pthread_mutex_t consume_mutex;
pthread_cond_t produce_signal;
pthread_cond_t consume_signal;
volatile bool updated = false;
volatile bool producing = true;
 
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
 
void* producer_routine(void* arg) {
    Value* value = static_cast<Value*>(arg);
    int n;

    while (std::cin >> n) {
        value->update(n);

        pthread_mutex_lock(&produce_mutex);
        updated = true;
        pthread_cond_signal(&produce_signal);
        pthread_mutex_unlock(&produce_mutex);

        pthread_mutex_lock(&consume_mutex);
        while (updated) {
            pthread_cond_wait(&consume_signal, &consume_mutex);
        }
        pthread_mutex_unlock(&consume_mutex);

    }
    producing = false;
    pthread_cond_signal(&produce_signal);
}
 
void* consumer_routine(void* arg) {
    Value* value = static_cast<Value*>(arg);
    int res = 0;
    int oldstate;

    while (producing) {

        pthread_mutex_lock(&produce_mutex);
        while (!updated && producing) {
            pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
            pthread_cond_wait(&produce_signal, &produce_mutex);
            pthread_setcancelstate(oldstate, &oldstate);
        }
        pthread_mutex_unlock(&produce_mutex);

        if (!producing) {
            break;
        }

        res += value->get();

        pthread_mutex_lock(&consume_mutex);
        updated = false;
        pthread_cond_signal(&consume_signal);
        pthread_mutex_unlock(&consume_mutex);

    }

    Value* result = new Value();
    result->update(res);
    return static_cast<void*>(result);
}
 
void* consumer_interruptor_routine(void* arg) {
    pthread_t* consumer = static_cast<pthread_t*>(arg);
    while(producing) {
        pthread_cancel(*consumer);
    }
}
 
int run_threads() {
    Value* value = new Value();
    pthread_t producer, consumer, interruptor;

    pthread_cond_init(&produce_signal, NULL);
    pthread_cond_init(&consume_signal, NULL);
    pthread_create(&producer, NULL, producer_routine, static_cast<void*>(&value));
    pthread_create(&consumer, NULL, consumer_routine, static_cast<void*>(&value));
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, static_cast<void*>(&value));
    pthread_join(interruptor, NULL);
    pthread_join(producer, NULL);
    pthread_join(consumer, reinterpret_cast<void**>(&value));

    return value->get();
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}