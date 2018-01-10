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

pthread_mutex_t producer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t consumer_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_signal = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_signal = PTHREAD_COND_INITIALIZER;

// For disable spurious wakeups
bool isReady = false;
bool isEnd = false;

void* producer_routine(void* arg) 
{
  Value *value = (Value*)arg;
  int n;

  while(cin >> n) 
  {
    value->update(n);
    pthread_mutex_lock(&producer_mutex);
    isReady = true;

    pthread_cond_signal(&producer_signal);
    pthread_mutex_unlock(&producer_mutex);
    pthread_mutex_lock(&consumer_mutex);

    while (isReady)
	{
        pthread_cond_wait(&consumer_signal, &consumer_mutex);
    }

    pthread_mutex_unlock(&consumer_mutex);
  }
  isEnd = true;
  pthread_cond_signal(&producer_signal);
}

void* consumer_routine(void* arg) 
{
  // Disable cancelling
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  Value *value = (Value*)arg;
  
  // Allocate value for result
  int *sum = new int;
  *sum = 0;

  while(!isEnd)
  {
    pthread_mutex_lock(&producer_mutex);
    while (!isReady && !isEnd) 
	{
         pthread_cond_wait(&producer_signal, &producer_mutex);
	}

    pthread_mutex_unlock(&producer_mutex);
    if (isEnd) 
	{
        break;
	}

    *sum = *sum + value->get();

    pthread_mutex_lock(&consumer_mutex);
    isReady = false;

    pthread_cond_signal(&consumer_signal);
    pthread_mutex_unlock(&consumer_mutex);  
  }
  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  return sum;
}

void* consumer_interruptor_routine(void* arg) 
{
  while (!isEnd) 
  {
    pthread_cancel(*(pthread_t*)arg);
  }
}

int run_threads() {
  Value value;
  int *sum;
  pthread_t producer_thread;
  pthread_t consumer_thread;
  pthread_t consumer_interruptor_thread;
  
  // start 3 threads and wait until they're done
  pthread_create(&producer_thread, NULL, producer_routine, &value);
  pthread_create(&consumer_thread, NULL, consumer_routine, &value);
  pthread_create(&consumer_interruptor_thread, NULL, consumer_interruptor_routine, &consumer_thread);

  // return sum of update values seen by consumer
  pthread_join(producer_thread, NULL);
  pthread_join(consumer_thread, (void**)&sum);

  int res = *sum;
  delete sum;
  return res;
}

int main() {
    cout << run_threads() << endl;
    return 0;
}
