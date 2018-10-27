#include <iostream>
#include <pthread.h>
#include <stdlib.h>
#include <vector>
#include <string>
#include <sstream>

#define VALGRIND_CHECK 0

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

// Notify producer about start
pthread_mutex_t tmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t tcond = PTHREAD_COND_INITIALIZER;
bool t_ready = false;
// Notify consumers about value update
pthread_mutex_t vmutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t vcond = PTHREAD_COND_INITIALIZER;
bool v_ready = false;

bool end = false;
unsigned max_threads;

void *producer_routine(void *arg)
{
    pthread_mutex_lock(&tmutex);
    while (!t_ready)
    {
        #if !VALGRIND_CHECK
        std::cout << "[Producer, TMUTEX] wait for a signal " << std::endl;
        #endif
        pthread_cond_wait(&tcond, &tmutex);
        #if !VALGRIND_CHECK
        std::cout << "[Producer, TMUTEX] receive a signal " << std::endl;
        #endif
    }
    // assume consumers to proccess the value so they are not ready
    t_ready = false;
    pthread_mutex_unlock(&tmutex);

    Value *value_instance = reinterpret_cast<Value *>(arg);
    // std::cout << "[producer]Value: " << value_instance->get() << std::endl;
    std::vector<int> input_values;
    // Wait for consumer to start
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
        // consumers are ready, update the value
        pthread_mutex_lock(&vmutex);
        value_instance->update(val);
        #if !VALGRIND_CHECK
        std::cout << "[Producer, VMUTEX] Broadcasting value: " << val << std::endl;
        #endif
        pthread_cond_broadcast(&vcond);
        v_ready = true;
        pthread_mutex_unlock(&vmutex);
        // wait for consumers
        pthread_mutex_lock(&tmutex);
        #if !VALGRIND_CHECK
        std::cout << "[Producer, TMUTEX]t_ready=" << (t_ready ? "true" : "false") << std::endl;
        #endif
        do
        {
            #if !VALGRIND_CHECK
            std::cout << "[Producer, TMUTEX] wait for a signal " << std::endl;
            #endif
            pthread_cond_wait(&tcond, &tmutex);
            #if !VALGRIND_CHECK
            std::cout << "[Producer, TMUTEX] receive a signal " << std::endl;
            #endif
        } while (!t_ready);
        // assume consumers to proccess the value so they are not ready
        // t_ready = false;
        pthread_mutex_unlock(&tmutex);
    }
    pthread_mutex_lock(&vmutex);
    end = true;
    pthread_cond_broadcast(&vcond);
    #if !VALGRIND_CHECK
    std::cout << "[Producer] Broadcast end" << std::endl;
    #endif
    pthread_mutex_unlock(&vmutex);

    pthread_exit(nullptr);
}

void *consumer_routine(void *arg)
{
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result
    static unsigned inc = 0;
    Value *value_instance = reinterpret_cast<Value *>(arg);
    // std::cout << "[consumer]Value: " << value_instance->get() << std::endl;

    Value *ret_val = new Value();
    while (1)
    {
        // barrier for all consumers
        pthread_mutex_lock(&tmutex);
        inc++;
        if (inc < max_threads)
        {
            #if !VALGRIND_CHECK
            std::cout << "[Consumer" << pthread_self() << ", TMUTEX]wait for other consumers, inc:"<< inc << std::endl;
            #endif
            while (!t_ready)
            {
                pthread_cond_wait(&tcond, &tmutex);
            }
            #if !VALGRIND_CHECK
            std::cout << "[Consumer" << pthread_self() << ", TMUTEX]other consumers are ready" << std::endl;
            #endif
        }
        else
        {
            #if !VALGRIND_CHECK
            std::cout << "[Consumer" << pthread_self() << ", TMUTEX]broadcasting, inc: " << inc << std::endl;
            #endif
            // consumers are ready, notify the producer
            t_ready = true;
            pthread_cond_broadcast(&tcond);
            inc = 0;
        }
        pthread_mutex_unlock(&tmutex);
        // wait for data and update it
        pthread_mutex_lock(&vmutex);
        do
        {
            #if !VALGRIND_CHECK
            std::cout << "[Consumer" << pthread_self() << ", VMUTEX]wait for value" << std::endl;
            #endif
            pthread_cond_wait(&vcond, &vmutex);
            if (end)
            {
                // Not spirious wake-up
                #if !VALGRIND_CHECK
                std::cout << "[Consumer" << pthread_self() << ", VMUTEX] on exit" << std::endl;
                #endif
                pthread_exit(reinterpret_cast<void *>(ret_val));
            }
        } while (!v_ready);
        #if !VALGRIND_CHECK
        std::cout << "[Consumer" << pthread_self() << ", VMUTEX]get the value: " << value_instance->get() << std::endl;
        #endif
        ret_val->update(value_instance->get() + ret_val->get());
        // invalidate the value
        // v_ready = false;
        t_ready = false;
        // inc = 0;
        pthread_mutex_unlock(&vmutex);
    }
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
