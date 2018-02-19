#include <pthread.h>
#include <iostream>
#include <vector>
#include <ctype.h>
#include <sstream>


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


pthread_t producer;
pthread_t consumer;
pthread_t interruptor;

pthread_mutex_t mutex;
pthread_cond_t condition;

volatile bool isUpdateValue = false;
volatile bool isFinish = false;

std::vector<int> list;

void readData()
{
    int num;

    std::string data;
    std::getline(std::cin, data);
    std::istringstream stream(data);

    while (stream >> num)
        list.push_back(num);   
}


void* producer_routine(void* arg)
{
    readData();

    std::vector<int>::iterator it = list.begin();
    while (it != list.end())
    {
        pthread_mutex_lock(&mutex);
        if (isUpdateValue)
            pthread_cond_wait(&condition, &mutex);

        ((Value*)arg)->update(*it);
        std::cout << "[PROD] value = " << *it << '\n';
        isUpdateValue = true;
        ++it;
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    isFinish = true;
    pthread_mutex_unlock(&mutex);
}

void* consumer_routine(void* arg)
{
    int* sum = new int;
    *sum = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    while (!isFinish)
    {
        pthread_mutex_lock(&mutex);
        if (!isUpdateValue)
                     pthread_cond_wait(&condition, &mutex);

        if (isUpdateValue)
        {
            *sum += ((Value*)arg)->get();
            std::cout << "[CONS] sum = " << *sum << '\n';
            isUpdateValue = false;
        }
        pthread_cond_signal(&condition);
        pthread_mutex_unlock(&mutex);
    }

    return (void*)sum;
}

void* consumer_interruptor_routine(void* arg)
{
    while (!isFinish)
        pthread_cancel(*(pthread_t*) arg);
}

int run_threads()
{
  Value* value = new Value();
  int* sum;

  pthread_mutex_init(&mutex, NULL);
  pthread_cond_init(&condition, NULL);

  pthread_create(&consumer, NULL, consumer_routine, value);
  pthread_create(&producer, NULL, producer_routine, value);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumer);

  pthread_join(producer, NULL);
  pthread_join(consumer, (void**)&sum);
  pthread_join(interruptor, NULL);

  pthread_mutex_destroy(&mutex);
  pthread_cond_destroy(&condition);

  delete value;
  return *sum;
}

int main()
{
  std::cout << run_threads() << std::endl;
  return 0;
}
