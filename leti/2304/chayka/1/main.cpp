#include <pthread.h>
#include <iostream>

volatile bool isNeedMore = false;
volatile bool isReady = false;
volatile bool isFin = false;

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
    while(!isNeedMore){}
    isNeedMore = false;
    value->update(n);
    isReady = true;
  }
  isFin = true;
}

void* consumer_routine(void* arg) {
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  Value *value = (Value*)arg;
  int *sum = new int;
  *sum = 0;
  isNeedMore = true;

  while(!isFin){
    while (!isReady) {
      if(isFin)
        return sum;
    }
    isReady = false;
    *sum += value->get();
    isNeedMore = true;
  }
  return sum;
}

void* consumer_interruptor_routine(void* arg) {
  while(!isNeedMore){}
  while (!isFin) {
    pthread_cancel(*(pthread_t*)arg);
  }
}

int run_threads() {
  pthread_t producer_thread;
  pthread_t consumer_thread;
  pthread_t consumer_interruptor_thread;
  Value value;
  int *sum;

  pthread_create(&producer_thread, NULL, producer_routine, &value);
  pthread_create(&consumer_thread, NULL, consumer_routine, &value);
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, (void**)&sum);

  int res = *sum;
  delete sum;
  return res;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
