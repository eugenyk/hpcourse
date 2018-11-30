#include <pthread.h>
#include <iostream>


volatile bool isReady = false;
volatile bool isFin = false;

pthread_mutex_t producer_mutex;
pthread_mutex_t consumer_mutex;
pthread_cond_t producer_signal;
pthread_cond_t consumer_signal;

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

 int count_numerous;
 
 Value *value = (Value*)arg;

  while(std::cin >> count_numerous) { 
      
          value->update(count_numerous);

          pthread_mutex_lock(&producer_mutex);
          
          isReady = true; 

         

          pthread_cond_signal(&producer_signal);
          pthread_mutex_unlock(&producer_mutex);

          pthread_mutex_lock(&consumer_mutex);
          
          while (isReady) { 
            pthread_cond_wait(&consumer_signal, &consumer_mutex); 
		}
          
          pthread_mutex_unlock(&consumer_mutex); /
        }

        isFin = true; 
        
        pthread_cond_signal(&producer_signal); 

}
 
void* consumer_routine(void* arg) {

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    
    Value *value = (Value*)arg; 
    
    int *summa = new int; 
    *summa = 0; 

    while(!isFin){
      pthread_mutex_lock(&producer_mutex);
      while (!isReady && !isFin) {
        pthread_cond_wait(&producer_signal, &producer_mutex);
      }
      pthread_mutex_unlock(&producer_mutex);
      if (isFin) {
          break;
      }

      *summa += value->get();

      pthread_mutex_lock(&consumer_mutex);
      isReady = false;
      pthread_cond_signal(&consumer_signal);
      pthread_mutex_unlock(&consumer_mutex);  
    }
    return summa;
}
 
void* consumer_interruptor_routine(void* arg) {

  while (!isFin) {
    pthread_cancel(*(pthread_t*)arg);
  }                                          
}
 
int run_threads() {
  
  pthread_t producer_thread;
  pthread_t consumer_thread;
  pthread_t consumer_interruptor_thread;
  pthread_cond_init(&producer_signal, NULL);
  pthread_cond_init(&consumer_signal, NULL);
  Value value;
  int *summa;

  pthread_create(&producer_thread, NULL, producer_routine, &value);
  pthread_create(&consumer_thread, NULL, consumer_routine, &value);
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, (void**)&summa);

  int res = *summa;
  delete summa;
  return res;
  return 0;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
