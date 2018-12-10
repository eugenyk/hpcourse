#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>
#include <time.h>
class Value
{
  public:
    Value() : _value(0) {}
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
pthread_cond_t tcond = PTHREAD_COND_INITIALIZER;
volatile bool t_ready = false;
pthread_mutex_t vmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t vcond = PTHREAD_COND_INITIALIZER;
volatile bool v_ready = false;
volatile bool end = false;
unsigned n_threads;
unsigned m_sec;
void *producer_routine(void *arg)
{
    pthread_mutex_lock(&vmutex);
    while (!t_ready)
    {
        pthread_cond_wait(&tcond, &vmutex);
    }
    pthread_mutex_unlock(&vmutex);
    Value *value_instance = reinterpret_cast<Value *>(arg);
    std::vector<int> input_values;
    std::string buf;
    std::getline(std::cin, buf);
    std::istringstream ss(buf);
    int input;
    while (ss >> input)
    {
        input_values.push_back(input);
    }
     for (int val: input_values)
    {
        pthread_mutex_lock(&vmutex);
        value_instance->update(val);
        v_ready = true;
        pthread_cond_broadcast(&vcond);
        do
        {
            pthread_cond_wait(&tcond, &vmutex);
        } while (!t_ready);
        t_ready = false;
        v_ready = false;
        pthread_mutex_unlock(&vmutex);
    }
    pthread_mutex_lock(&vmutex);
    end = true;
    pthread_cond_broadcast(&vcond);
    v_ready = true;
    pthread_mutex_unlock(&vmutex);
    pthread_exit(nullptr);
}
 void *consumer_routine(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    static int count = 0;
    Value *value_instance = reinterpret_cast<Value *>(arg);
    Value *ret_val = new Value();
     while (1)
    {
        pthread_mutex_lock(&vmutex);
        count ++;
        if (count >= n_threads)
        {
            v_ready = false;
            t_ready = true;
            pthread_cond_broadcast(&tcond);
            count = 0;
        }
        do
        {
            pthread_cond_wait(&vcond, &vmutex);
        } while (!v_ready);
        t_ready = false;
         if (!end)
        {
            ret_val->update(value_instance->get() + ret_val->get());
        }
        else
        {
            pthread_mutex_unlock(&vmutex);
            pthread_exit(reinterpret_cast<void *>(ret_val));
        }
        pthread_mutex_unlock(&vmutex);
        unsigned time_to_sleep = (m_sec ? (rand() % m_sec) : 0);
        timespec ts;
        ts.tv_sec = (float)time_to_sleep / 1000.0;
        ts.tv_nsec = 0;
        nanosleep(&ts, nullptr);
    }
}
 void *consumer_interruptor_routine(void *arg)
{
    pthread_t *consumers = reinterpret_cast<pthread_t *>(arg);
    while (!end)
    {
        unsigned i = rand() % n_threads;
        pthread_cancel(consumers[i]);
    }
    pthread_exit(nullptr);
}
 int run_threads(unsigned threads_cnt, unsigned msec)
{
    Value *value_instance = new Value();
    Value *ret_val;
    n_threads = threads_cnt;
    m_sec = msec;
    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value_instance);
    pthread_t *consumers = new pthread_t[n_threads];
    for (size_t i = 0; i < n_threads; i++)
    {
        pthread_create(&consumers[i], nullptr, consumer_routine, value_instance);
    }
    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, consumers);
    for (size_t i = 0; i < n_threads; i++)
    {
        pthread_join(consumers[i], reinterpret_cast<void **>(&ret_val));
    }
    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    delete[] consumers;
    return ret_val->get();
}

int main(int argc, const char *argv[])
{
using namespace std;
    int threads_cnt, sleep_msec;
    if (argc <= 2)
    {   
        printf("Need more arguments");
        return 1;
    }
    threads_cnt = atoi(argv[1]);
    sleep_msec = atoi(argv[2]);
    std::cout << run_threads((unsigned)threads_cnt, (unsigned)sleep_msec) << std::endl;
    return 0;
}
