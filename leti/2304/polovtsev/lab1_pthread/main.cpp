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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_condition = PTHREAD_COND_INITIALIZER;
pthread_cond_t cons_start_condition = PTHREAD_COND_INITIALIZER;

bool startProducer = false;
bool startConsumer = false;
bool startInterruptor = false;
bool isUpdateValue = false;
bool isFinish = false;

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

    pthread_mutex_lock(&mutex);
    while (!startConsumer)
        pthread_cond_wait(&cons_start_condition, &mutex);
    startProducer = true;
    pthread_mutex_unlock(&mutex);


    for (std::vector<int>::iterator it = list.begin();it != list.end();++it)
    {
        pthread_mutex_lock(&mutex);
        ((Value*)arg)->update(*it);
        std::cout << "[PROD] value = " << *it << '\n';
        isUpdateValue = true;
        pthread_cond_signal(&consumer_condition);
        while (isUpdateValue)
            pthread_cond_wait(&producer_condition, &mutex);
        pthread_mutex_unlock(&mutex);
    }

    isFinish = true;
    isUpdateValue = true;
    pthread_cond_signal(&consumer_condition);
}

void* consumer_routine(void* arg)
{
    int* sum = new int;
    *sum = 0;

    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    startConsumer = true;
    while (!startProducer && !startInterruptor)
        pthread_cond_signal(&cons_start_condition);

    while (!isFinish)
    {
        pthread_mutex_lock(&mutex);
        if (isUpdateValue)
        {
            *sum += ((Value*)arg)->get();
            std::cout << "[CONS] sum = " << *sum << '\n';
            isUpdateValue = false;
            pthread_cond_signal(&producer_condition);
        }
        while (!isUpdateValue)
            pthread_cond_wait(&consumer_condition, &mutex);
        pthread_mutex_unlock(&mutex);
    }

    return (void*)sum;
}

void* consumer_interruptor_routine(void* arg)
{
    while (true)
    {
        if (startConsumer) break;
    }
    startInterruptor = true;
    while (!isFinish)
        pthread_cancel(*(pthread_t*) arg);
}

int run_threads()
{
  Value* value = new Value();
  int* sum;

  pthread_create(&consumer, NULL, consumer_routine, value);
  pthread_create(&producer, NULL, producer_routine, value);
  pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumer);

  pthread_join(producer, NULL);
  pthread_join(consumer, (void**)&sum);
  pthread_join(interruptor, NULL);


  delete value;
  return *sum;
}

int main()
{
  std::cout << run_threads() << std::endl;
  return 0;
}
