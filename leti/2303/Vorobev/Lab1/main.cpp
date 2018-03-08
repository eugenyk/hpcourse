#include <iostream>
#include <pthread.h>
#include <iostream>

using namespace std;
 
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

pthread_t producer;
pthread_t consumer;
pthread_t interrupter;
pthread_mutex_t mutex;
pthread_cond_t cond;


int condition = 0;
bool consumer_starts=false;
bool continue_updates = true;

 
void* producer_routine(void* arg) {

    while (!consumer_starts)
    {

    }
    int n;   
    int x;   
    cout << "Enter the number of values: ";
    cin >> n;
    cout << "Enter the values: ";
    Value *value = (Value*)arg;

    while( n > 0 ) 
    { 
        pthread_mutex_lock( &mutex ); 
        while( condition == 1 ) 
            pthread_cond_wait( &cond, &mutex ); 
        cin >> x;
        value->update(x);
        n--;
        condition = 1; 
        pthread_cond_signal( &cond ); 
        pthread_mutex_unlock( &mutex ); 
    } 
    continue_updates = false;
    
    
}
 
void* consumer_routine(void* arg) {

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    consumer_starts = true;
    int *sum = new int();
    Value *value = (Value*)arg;
    *sum = 0;
    while(continue_updates) 
    { 
        
        pthread_mutex_lock( &mutex ); 
        while( condition == 0 ) 
            pthread_cond_wait( &cond, &mutex ); 
        *sum += value->get();
        condition = 0; 
        pthread_cond_signal( &cond ); 
        pthread_mutex_unlock( &mutex ); 
    }   
    return sum;
}
 
void* consumer_interruptor_routine(void* arg) {

    while (!consumer_starts)
    {

    }
 
    while(continue_updates)
    {
        pthread_cancel(consumer);
    }                                       
}
 
int run_threads() {

    Value *value = new Value();
    int *sum;
    

    pthread_create(&producer,NULL,&producer_routine,value);
    pthread_create(&consumer,NULL,&consumer_routine,value);
    pthread_create(&interrupter,NULL,&consumer_interruptor_routine,NULL);


    pthread_join(producer,NULL);
    pthread_join(consumer,(void**)&sum); 
    pthread_join(interrupter,NULL);
    
    int ans = *sum;
    delete sum;
    return ans;
}
 
int main() {
    cout << run_threads() << endl;
    return 0;
}