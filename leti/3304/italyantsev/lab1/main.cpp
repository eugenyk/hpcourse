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

pthread_barrier_t mybarrier_w;
pthread_barrier_t mybarrier_n;
pthread_barrier_t mybarrier_u;
volatile bool end = false;
unsigned n_threads;
unsigned m_sec;

void *producer_routine(void *arg)
{
    Value *value_instance = reinterpret_cast<Value *>(arg);
    pthread_barrier_wait(&mybarrier_w);

    std::string buf;
    std::getline(std::cin, buf);
    std::istringstream ss(buf);
    int input;
    while (ss >> input)
    {
        value_instance->update(input);
        pthread_barrier_wait(&mybarrier_n);
        pthread_barrier_wait(&mybarrier_u);
    }
    end = true;
    pthread_barrier_wait(&mybarrier_n);
    pthread_exit(nullptr);
}

void *consumer_routine(void *arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    pthread_barrier_wait(&mybarrier_w);
    Value *value_instance = reinterpret_cast<Value *>(arg);
    Value *ret_val = new Value();
    while (1)
    {
        pthread_barrier_wait(&mybarrier_n);
        if (!end)
        {
            ret_val->update(value_instance->get() + ret_val->get());
        }
        else
        {
            pthread_exit(reinterpret_cast<void *>(ret_val));
        }
        unsigned time_to_sleep = (m_sec ? (rand() % m_sec) : 0);
        timespec ts;
        ts.tv_sec = (float)time_to_sleep / 1000.0;
        ts.tv_nsec = 0;
        nanosleep(&ts, nullptr);
        pthread_barrier_wait(&mybarrier_u);
    }
}
 void *consumer_interruptor_routine(void *arg)
{
    pthread_t *consumers = reinterpret_cast<pthread_t *>(arg);      
    pthread_barrier_wait(&mybarrier_w);    
    

    while (!end)
    {
        pthread_cancel(consumers[rand() % n_threads]);
    }
    
    pthread_exit(nullptr);
}
 int run_threads()
{
    pthread_barrier_init(&mybarrier_w, nullptr, static_cast<unsigned>(n_threads + 2));
    pthread_barrier_init(&mybarrier_n, nullptr, static_cast<unsigned>(n_threads + 1));
    pthread_barrier_init(&mybarrier_u, nullptr, static_cast<unsigned>(n_threads + 1));
    Value *value_instance = new Value();
    Value *ret_val;
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
    pthread_barrier_destroy(&mybarrier_w);
    pthread_barrier_destroy(&mybarrier_n);
    pthread_barrier_destroy(&mybarrier_u);
    return ret_val->get();
}

int main(int argc, const char *argv[])
{
using namespace std;
    if (argc <= 2)
    {   
        printf("Need more arguments");
        return 1;
    }
    n_threads = atoi(argv[1]);
    m_sec = atoi(argv[2]);
    std::cout << run_threads() << std::endl;
    return 0;
}
