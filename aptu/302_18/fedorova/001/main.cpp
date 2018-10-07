#include <iostream>
#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>

using namespace std;

pthread_mutex_t count_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  condition_var = PTHREAD_COND_INITIALIZER;

pthread_t thread_producer, thread_consumer, thread_interruptor;
volatile bool con_began = false, prod_finished = false, val_changed = false;

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
    int v;

    pthread_mutex_lock(&count_mutex);
    if (!con_began) {
        pthread_cond_wait(&condition_var, &count_mutex);
    }

    while (cin >> v) {
        while (val_changed) {
            pthread_cond_wait(&condition_var, &count_mutex);
        }
        (static_cast<Value *>(arg)) -> update(v);
        val_changed = true;
        pthread_cond_broadcast(&condition_var);
        pthread_cond_wait(&condition_var, &count_mutex);
    }
    prod_finished = true;
    pthread_cond_broadcast(&condition_var);
    pthread_mutex_unlock(&count_mutex);
    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    auto* sum = new int(0);
    pthread_mutex_lock(&count_mutex);
    con_began = true;
    pthread_cond_broadcast(&condition_var);
    while (!prod_finished) {
        pthread_cond_wait(&condition_var, &count_mutex);
        if (val_changed){
            *sum += (static_cast<Value *>(arg))->get();
            val_changed = false;
            pthread_cond_broadcast(&condition_var);
        }
    }
    con_began = false;
    pthread_mutex_unlock(&count_mutex);
    return sum;
}

void* consumer_interruptor_routine(void* arg) {
    pthread_mutex_lock(&count_mutex);
    if (!con_began) {
        pthread_cond_wait(&condition_var, &count_mutex);
    }
    pthread_mutex_unlock(&count_mutex);
    while (con_began) {
        pthread_cancel(thread_consumer);
    }
    return nullptr;
}

int run_threads() {
    int *result;
    Value* data = new Value();
    pthread_mutex_init(&count_mutex, nullptr);
    pthread_cond_init(&condition_var, nullptr);
    pthread_create(&thread_producer, NULL, producer_routine, data);
    pthread_create(&thread_consumer, NULL, consumer_routine, data);
    pthread_create(&thread_interruptor, NULL, consumer_interruptor_routine, NULL);

    pthread_join(thread_producer, nullptr);
    pthread_join(thread_consumer, (void **)&result);
    pthread_join(thread_interruptor, nullptr);
    return *result;
}

int main() {
    cout << run_threads() << endl;
    return 0;
}

