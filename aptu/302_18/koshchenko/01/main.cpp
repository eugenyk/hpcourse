#include <iostream>
#include <pthread.h>
 
pthread_cond_t condition;
pthread_mutex_t mutex;
pthread_t producer;
pthread_t consumer;
pthread_t interruptor;
 
int consumer_status = -1; //0 - started, 1 - ran out of values
int producer_status = -1; //0 - started, 1 - finished
 
class Value {
public:
    Value(): _value(0) {}
   
    void update(int value) {
        _value = value;
    }
   
    int get() const {
        return _value;
    }
private:
    int _value;
};
 
 
void waiting_consumer_working() {
    consumer_status = 0;
    pthread_cond_broadcast(&condition);
    while (consumer_status != 1) {
        pthread_cond_wait(&condition, &mutex);
    }
}
 
void waiting_consumer_starting() {
    pthread_mutex_lock(&mutex);
    while(consumer_status == -1) {
        pthread_cond_wait(&condition, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}
 
 
void* producer_routine(void* arg) {
    producer_status = 0;
    waiting_consumer_starting();
    Value *value = static_cast<Value*>(arg);
    int n;
    while (std::cin >> n) {
        pthread_mutex_lock(&mutex);
        value -> update(n);
        waiting_consumer_working();
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);
    producer_status = 1;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&mutex);
    return NULL;
}
 
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_mutex_lock(&mutex);
    consumer_status = 1;
    Value *value = static_cast<Value*>(arg);
    int *res = new int;
    *res = 0;
    pthread_cond_broadcast(&condition);
    pthread_mutex_unlock(&mutex);
    while (producer_status != 1) {
        pthread_mutex_lock(&mutex);
        while (producer_status == 0 && consumer_status == 1) {
            pthread_cond_wait(&condition, &mutex);
        }
        if (producer_status == 0) {
            *res += value->get();
            consumer_status = 1;
            pthread_cond_broadcast(&condition);
        }
        pthread_mutex_unlock(&mutex);
    }
    pthread_mutex_lock(&mutex);
    consumer_status = 1;
    pthread_mutex_unlock(&mutex);
    return res;
}
 
void* consumer_interruptor_routine(void* arg) {
    waiting_consumer_starting();
    while (producer_status == 0) {
        pthread_cancel(consumer);
    }
    return NULL;
}
 
int run_threads() {
    Value value;
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&condition, NULL);
    pthread_create(&producer, NULL, producer_routine, &value);
    pthread_create(&consumer, NULL, consumer_routine, &value);
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);
 
    int *res = NULL;
    pthread_join(producer, NULL);
    pthread_join(consumer, reinterpret_cast<void **>(&res));
    pthread_join(interruptor, NULL);
    pthread_cond_destroy(&condition);
    pthread_mutex_destroy(&mutex);
 
    return *res;
}
 
 
int main()
{
   std::cout << run_threads() << std::endl;
   return 0;
}
