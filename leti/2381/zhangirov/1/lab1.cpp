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



pthread_t producer_thread, consumer_thread, interruptor_thread;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

bool update_state = false;
bool finish_state = false;
bool consumer_started = false;
 
void* producer_routine(void* arg) {
    Value * val = (Value *)arg;
    int n;
  
    pthread_mutex_lock(&mutex);
  
    while(std::cin >> n){  
        //std::cout << "\nP:" << n;
        
        while(update_state){
            pthread_cond_wait(&cond, &mutex);
        }
        
        val->update(n);
        update_state = true;
    }
  
    while(update_state){
        pthread_cond_wait(&cond, &mutex);
    }
  
    finish_state = true;
    pthread_mutex_unlock(&mutex);
}
 
void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    
    Value *val = (Value *) arg;
    int* result = new int;
    *result = 0;
    
    consumer_started = true;
    
    while(!finish_state){
        pthread_mutex_lock(&mutex); 
        if(update_state){
            *result += val->get();
            //std::cout << "\nC:" << *result;
            update_state = false;
            pthread_cond_signal(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    return result;
}
 
void* consumer_interruptor_routine(void* arg) {
    while(!consumer_started){}
  
    while(!finish_state){
        pthread_cancel(consumer_thread);
    }
}
 
int run_threads() {
    Value val;
    int* sum;
    pthread_create(&producer_thread, NULL, producer_routine, (void*)&val);
    pthread_create(&consumer_thread, NULL, consumer_routine, (void*)&val);
    pthread_create(&interruptor_thread, NULL, consumer_interruptor_routine, NULL);
  
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, (void**)(&sum));
    pthread_join(interruptor_thread, NULL);
  
    return *sum;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
} 