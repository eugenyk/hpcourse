#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
bool t_ready = false;
bool end = false;
bool v_ready = false;
unsigned max_threads;

void *producer_routine(void *arg)
{
    Value *value_instance = reinterpret_cast<Value *>(arg);
    // std::cout << "[producer]Value: " << value_instance->get() << std::endl;
    std::vector<int> input_values;
    // Wait for consumer to start
    pthread_mutex_lock(&mutex);
    while (!t_ready)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
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
        pthread_mutex_lock(&mutex);
        v_ready = false;
        value_instance->update(val);
        pthread_cond_broadcast(&cond);
        v_ready = true;
        pthread_mutex_unlock(&mutex);
    }
    end = true;

    pthread_exit(nullptr);
}

void *consumer_routine(void *arg)
{
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result
    static unsigned inc = 0;
    pthread_mutex_lock(&mutex);
    inc++;
    if (inc < max_threads)
    {
        while (!t_ready)
        {
            pthread_cond_wait(&cond, &mutex);
        }
    }
    else
    {
        t_ready = true;
        inc = 0;
        pthread_cond_broadcast(&cond);
    }
    pthread_mutex_unlock(&mutex);

    Value *value_instance = reinterpret_cast<Value *>(arg);
    // std::cout << "[consumer]Value: " << value_instance->get() << std::endl;

    Value *ret_val = new Value();

    while (!end)
    {
        pthread_mutex_lock(&mutex);
        while (!v_ready)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        ret_val->update(value_instance->get() + ret_val->get());
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(reinterpret_cast<void *>(ret_val));
}

void *consumer_interruptor_routine(void *arg)
{
    // wait for consumer to start
    const unsigned msec = *reinterpret_cast<const unsigned *>(arg);
    // interrupt consumer while producer is running
    // std::cout << "[interruptor]sleep for " << msec << std::endl;

    pthread_exit(nullptr);
}

int run_threads(unsigned n_threads, unsigned msec)
{
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer
    Value *value_instance = new Value();
    Value *ret_val;
    max_threads = n_threads;
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
    if (argc < 3)
    {
        return EXIT_FAILURE;
    }
    n_threads = atoi(argv[1]);
    sleep_msec = atoi(argv[2]);

    std::cout << run_threads((unsigned)n_threads, (unsigned)sleep_msec) << std::endl;
    return 0;
}
