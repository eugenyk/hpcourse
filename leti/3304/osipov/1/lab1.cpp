#include <iostream>
#include <pthread.h>
#include <stdlib.h>

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

void *producer_routine(void *arg)
{
    Value *value_instance = reinterpret_cast<Value *>(arg);
    std::cout << "[producer]Value: " << value_instance->get() << std::endl;
    // Wait for consumer to start

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    pthread_exit(nullptr);
}

void *consumer_routine(void *arg)
{
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result
    Value *value_instance = reinterpret_cast<Value *>(arg);
    std::cout << "[consumer]Value: " << value_instance->get() << std::endl;

    Value *ret_val = new Value();
    ret_val->update(777);

    pthread_exit(reinterpret_cast<void *>(ret_val));
}

void *consumer_interruptor_routine(void *arg)
{
    // wait for consumer to start
    const unsigned msec = *reinterpret_cast<const unsigned *>(arg);
    // interrupt consumer while producer is running
    std::cout << "[interruptor]sleep for " << msec << std::endl;

    pthread_exit(nullptr);
}

int run_threads(unsigned n_threads, unsigned msec)
{
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer
    Value *value_instance = new Value();
    Value *ret_val;

    // creating of 2 + n_threads threads
    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value_instance);

    pthread_t *consumers = new pthread_t[n_threads];
    for (size_t i = 0; i < n_threads; i++)
    {
        pthread_create(&consumers[i], nullptr, consumer_routine, value_instance);
    }

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &msec);

    // waiting for threads' end
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
    int n_threads, sleep_msec;
    n_threads = atoi(argv[1]);
    sleep_msec = atoi(argv[2]);

    std::cout << run_threads((unsigned)n_threads, (unsigned)sleep_msec) << std::endl;
    return 0;
}
