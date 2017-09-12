#include <pthread.h>
#include <iostream>
#include <vector>

using namespace std;

class Value
{
public:
  Value() : _value(0) { }

  void update(int value)
  {
    _value = value;
  }

  int get() const
  {
    return _value;
  }

private:
  int _value;
};

pthread_mutex_t m;
pthread_cond_t upd;
bool is_new_val = true;
bool is_end = false;

pthread_t producer, consumer, interruptor;

void* producer_routine(void* arg)
{
  // Read data
  vector<int> data;
  int num;
  while (cin.peek() != '\n')
    {
      cin >> num;
      data.push_back(num);
    }

  //Loop through each value and update the value, notify consumer, wait for consumer to process
  for (vector<int>::iterator it = data.begin(); it != data.end(); ++it)
    {
      // wait for consumer to start / to process value
      pthread_mutex_lock(&m);
      while (is_new_val)
        pthread_cond_wait(&upd, &m);

      //update value
      ((Value*)arg)->update(*it);
      is_new_val = true;
      pthread_cond_signal(&upd);
      pthread_mutex_unlock(&m);
    }

  //signal to finish all tasks
  pthread_mutex_lock(&m);
  is_end = true;
  pthread_cond_signal(&upd);
  pthread_mutex_unlock(&m);
}

void* consumer_routine(void* arg)
{
  // allocate value for result
  int* result = new int;
  *result = 0;
  pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  // notify about start
  pthread_mutex_lock(&m);
  is_new_val = false;
  pthread_cond_signal(&upd);
  pthread_mutex_unlock(&m);

  while (!is_end)
    {
      //wait any signal
      pthread_mutex_lock(&m);
      while (!is_new_val && !is_end)
        pthread_cond_wait(&upd, &m);

      // for every update issued by producer, read the value and add to sum
      if (is_new_val)
        {
          *result += ((Value*)arg)->get();
          is_new_val = false;
          pthread_cond_signal(&upd);
        }

      pthread_mutex_unlock(&m);
    }

  // return pointer to result
  return (void*)result;
}

void* consumer_interruptor_routine(void* arg)
{
  // wait for consumer to start
  pthread_mutex_lock(&m);
  while (is_new_val)
    pthread_cond_wait(&upd, &m);
  pthread_mutex_unlock(&m);

  // try interrupt consumer while producer is running
  while (!is_end)
    pthread_cancel(consumer);
}

int run_threads()
{
  // start 3 threads and wait until they're done
  Value* val = new Value();
  int* sum = new int;

  pthread_create(&consumer, NULL, consumer_routine, (void*)val);
  pthread_create(&producer, NULL, producer_routine, (void*)val);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);

  pthread_join(producer, NULL);
  pthread_join(consumer, (void**)&sum);
  pthread_join(interruptor, NULL);

  // return sum of update values seen by consumer
  return *sum;
}

int main()
{
  cout << run_threads() << endl;
  return 0;
}
