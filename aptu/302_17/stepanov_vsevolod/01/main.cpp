#include <pthread.h>
#include <iostream>

const int CONSUMER_NOT_STARTED = 0;
const int WAIT_FOR_VALUE_WRITE = 1;
const int WAIT_FOR_VALUE_READ = 2;
const int CONSUMER_FINISHED = 3;

int state = CONSUMER_NOT_STARTED;

pthread_cond_t cond;
pthread_mutex_t mutex;

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
    Value *value = (Value*) arg;

    pthread_mutex_lock(&mutex);
    while (state == CONSUMER_NOT_STARTED) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    int n;
    std::cin >> n;
    for (int i = 0; i < n; i++) {
        int num;
        std::cin >> num;

        pthread_mutex_lock(&mutex);
        while (state != WAIT_FOR_VALUE_WRITE) {
            pthread_cond_wait(&cond, &mutex);
        }

        value->update(num);
        state = i == n - 1 ? CONSUMER_FINISHED : WAIT_FOR_VALUE_READ;

        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    }


    return nullptr;
}
 
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    pthread_mutex_lock(&mutex);
    state = WAIT_FOR_VALUE_WRITE;
    pthread_mutex_unlock(&mutex);

    pthread_cond_broadcast(&cond);

    int *sum = new int(0);
    Value *value = (Value*) arg;

    while (1) {
        pthread_mutex_lock(&mutex);
        while (state == WAIT_FOR_VALUE_WRITE) {
            pthread_cond_wait(&cond, &mutex);
        }
        
        if (state == CONSUMER_FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        *sum += value->get();
        state = WAIT_FOR_VALUE_WRITE;
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond);
    }
    
    return (void*) sum;
}
 
void* consumer_interruptor_routine(void* arg) {
    pthread_t consumer_thread = *((pthread_t*) arg);

    pthread_mutex_lock(&mutex);
    while (state == CONSUMER_NOT_STARTED) {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);

    int cur_state = state;
    while (cur_state != CONSUMER_FINISHED) {
        pthread_cancel(consumer_thread);

        pthread_mutex_lock(&mutex);
        cur_state = state;
        pthread_mutex_unlock(&mutex);
    }

    return nullptr;
}
 
int run_threads() {
    Value value;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL); 
    
    pthread_t producer_thread;
    pthread_t consumer_thread;
    pthread_t interruptor_thread;

    pthread_create(&producer_thread, NULL, producer_routine, &value);
    pthread_create(&consumer_thread, NULL, consumer_routine, &value);
    pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread); 

    void *sum;
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, &sum);
    pthread_join(interruptor_thread, NULL);
    int result = *((int*) sum);

    delete (int*) sum;
    pthread_cond_destroy(&cond);
    pthread_mutex_destroy(&mutex);
    return result;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}

