#include <pthread.h>
#include <iostream>

bool Start = false;
bool Finish = false;

pthread_t producer_thread;
pthread_t consumer_thread;
pthread_t consumer_interruptor_thread;

pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumer_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

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
  int n;
  Value *value = (Value*)arg;

  while(std::cin >> n) {
    value->update(n);
    pthread_mutex_lock(&producer_mutex);
    Start = true;
    pthread_cond_signal(&producer_cond);
    pthread_mutex_unlock(&producer_mutex);

    pthread_mutex_lock(&consumer_mutex);
    while (Start) {
        pthread_cond_wait(&consumer_cond, &consumer_mutex);
    }
    pthread_mutex_unlock(&consumer_mutex);
  }
  
  Finish = true;

  pthread_cond_signal(&producer_cond);
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  
  Value *value = (Value*)arg;
  int *sum = new int;
  *sum = 0;

  while(!Finish){
    pthread_mutex_lock(&producer_mutex);
     while (!Start && !Finish) {
         pthread_cond_wait(&producer_cond, &producer_mutex);
    }
    pthread_mutex_unlock(&producer_mutex);
    if (Finish) {
        break;
    }

    *sum += value->get();

    pthread_mutex_lock(&consumer_mutex);
    Start = false;
    pthread_cond_signal(&consumer_cond);
    pthread_mutex_unlock(&consumer_mutex);  }
  
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  
  return sum;
}

void* consumer_interruptor_routine(void* arg) {
  while (!Finish) {
    pthread_cancel(consumer_thread);
  }
}

int run_threads() {
  Value value;
  int *result;

  pthread_create(&producer_thread, NULL, producer_routine, &value);
  pthread_create(&consumer_thread, NULL, consumer_routine, &value);
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, NULL);

  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, (void**)&result);
  pthread_join(consumer_interruptor_thread, NULL);

  return *result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}