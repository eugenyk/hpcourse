#include <pthread.h>
#include <iostream>
#include <vector>
#include <limits.h>

using namespace std;

bool dbg = true; //show messages in console

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
bool is_new_val = true; //if FALSE producer can push new value
bool is_end = false; //flag shows end of process

pthread_t producer, consumer, interruptor;

void lock_and_wait(){
    int rtn = 0; //flag indicates error in cond_wait
    pthread_mutex_lock(&m);
    while (is_new_val || rtn!=0)
      rtn = pthread_cond_wait(&upd, &m);
}

void signal_and_unlock(){
    pthread_cond_signal(&upd);
    pthread_mutex_unlock(&m);
}


void* producer_routine(void* arg)
{
  // Read data
  vector<int> data;
  while (cin.peek() != '\n')
    {
      int num = INT_MIN;
      cin >> num;
      if (num == INT_MIN) {
          cout << "Wrong input" << endl;
          break;
      }
      data.push_back(num);
    }

  //Loop through each value and update the value, notify consumer, wait for consumer to process
  for (vector<int>::iterator it = data.begin(); it != data.end(); ++it)
    {
      // wait for consumer to start / to process value
      lock_and_wait();
      //update value
      ((Value*)arg)->update(*it);
      is_new_val = true;
      if (dbg) cout << "producer: next value = " << *it << endl;
      signal_and_unlock();
    }

  //wait for consumer to end all tasks
  lock_and_wait();
  //set flag to finish consumer
  is_end = true;
  if (dbg) cout << "producer: finished\n";
  signal_and_unlock();

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
  if (dbg) cout << "consumer: started\n";
  signal_and_unlock();

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
          if (dbg) cout << "consumer: added value = " << ((Value*)arg)->get() << ", sum = " << *result << endl;
        }

      signal_and_unlock();
    }

  if (dbg) cout << "consumer: finished\n";

  // return pointer to result
  return (void*)result;
}

void* interruptor_routine(void* arg)
{
  // wait for consumer to start
  lock_and_wait();
  if (dbg) cout << "interruptor: started\n";
  signal_and_unlock();

  // try interrupt consumer while producer is running
  while (!is_end)
    pthread_cancel(consumer);

  if (dbg) cout << "interruptor: finished\n";
}

int run_threads()
{
  // start 3 threads and wait until they're done
  Value* val = new Value();
  int* sum = new int;
  pthread_cond_init(&upd, NULL);

  pthread_create(&consumer, NULL, consumer_routine, (void*)val);
  pthread_create(&producer, NULL, producer_routine, (void*)val);
  pthread_create(&interruptor, NULL, interruptor_routine, NULL);

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
