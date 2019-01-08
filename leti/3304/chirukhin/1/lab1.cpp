#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <pthread.h>
#include <unistd.h>

// from argv
int num_of_consumers;
int sleep_time;

int num_of_consumers_started = 0;
int num_of_consumers_updated = 0;

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t all_consumers_started_cond = PTHREAD_COND_INITIALIZER;
bool consumers_started = false;

pthread_cond_t consumers_done_update_cond = PTHREAD_COND_INITIALIZER;
bool consumers_updated = false;

pthread_cond_t producer_updated_value_cond = PTHREAD_COND_INITIALIZER;
int producer_update_count = 0;

bool done = false;

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

struct producer_args
{
    producer_args(Value* _value, std::vector<int> _numbers)
        : value(_value), numbers(_numbers) { }

    Value* value;
    std::vector<int> numbers;
};

struct consumer_args
{
    consumer_args() : sum(0) { }

    Value* value;
    int sum;
};

struct interruptor_args
{
    interruptor_args(std::vector<pthread_t> _consumers)
        : consumers(_consumers) { }

    std::vector<pthread_t> consumers;
};

void* producer_routine(void* arg)
{
    producer_args* args = (producer_args*)arg;

    pthread_mutex_lock(&value_mutex);

    // wait for consumers to start
    while (!consumers_started)
    {
        pthread_cond_wait(&all_consumers_started_cond, &value_mutex);
    }

    std::vector<int>:: iterator it;
    for (it = args->numbers.begin(); it != args->numbers.end(); it++)
    {
        (args->value)->update(*it);
        producer_update_count++;
        consumers_updated = false;

        // notify consumers
        pthread_cond_broadcast(&producer_updated_value_cond);

        // wait for consumers to update sum
        while (!consumers_updated)
        {
            pthread_cond_wait(&consumers_done_update_cond, &value_mutex);
        }
    }

    done = true;

    pthread_mutex_unlock(&value_mutex);

    // notify about exit
    pthread_cond_broadcast(&producer_updated_value_cond);
}

void* consumer_routine(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    consumer_args* args = (consumer_args*)arg;

    // count started consumers
    pthread_mutex_lock(&value_mutex);
    num_of_consumers_started++;
    if (num_of_consumers_started == num_of_consumers)
    {
        consumers_started = true;
        pthread_cond_broadcast(&all_consumers_started_cond);
    }
    pthread_mutex_unlock(&value_mutex);

    int local_update_count = 0;

    for (;;)
    {
        // wait for producer
        pthread_mutex_lock(&value_mutex);
        while ((producer_update_count == local_update_count) && !done)
        {
            pthread_cond_wait(&producer_updated_value_cond, &value_mutex);
        }

        if (done)
        {
            break;
        }

        args->sum += (args->value)->get();
        local_update_count++;

        // count consumers that updated sum
        num_of_consumers_updated++;
        if (num_of_consumers_updated == num_of_consumers)
        {
            num_of_consumers_updated = 0;
            consumers_updated = true;
            pthread_cond_signal(&consumers_done_update_cond);
        }

        pthread_mutex_unlock(&value_mutex);

        // sleep for a random time
        int rnd_sleep_time = 0;
        if (sleep_time != 0)
        {
            rnd_sleep_time = rand() % sleep_time;
        }
        usleep(rnd_sleep_time * 1000);
    }

    pthread_mutex_unlock(&value_mutex);
}

void* consumer_interruptor_routine(void* arg)
{
    interruptor_args* args = (interruptor_args*)arg;

    // wait for consumers to start
    pthread_mutex_lock(&value_mutex);
    while (!consumers_started)
    {
        pthread_cond_wait(&all_consumers_started_cond, &value_mutex);
    }
    pthread_mutex_unlock(&value_mutex);

    while (!done)
    {
        pthread_cancel(args->consumers[rand() % num_of_consumers]);
    }
}

int run_threads()
{
    // read numbers from input stream
    std::vector<int> numbers;
    std::string buffer;
    std::getline(std::cin, buffer);
    std::istringstream iss(buffer);
    int i;

    while (iss >> i)
    {
        numbers.push_back(i);
    }

    // shared value
    Value* value = new Value();

    // create producer
    pthread_t producer;
    producer_args p_args(value, numbers);
    pthread_create(&producer, NULL, &producer_routine, &p_args);

    // create N consumers
    std::vector<pthread_t> consumers(num_of_consumers);
    std::vector<consumer_args> c_args(num_of_consumers);
    for (i = 0; i < num_of_consumers; i++)
    {
        c_args[i].value = value;
        pthread_create(&consumers[i], NULL, &consumer_routine, &c_args[i]);
    }

    // create interruptor
    pthread_t interruptor;
    interruptor_args i_args(consumers);
    pthread_create(&interruptor, NULL, &consumer_interruptor_routine, &i_args);

    // wait for threads
    pthread_join(producer, NULL);
    for (i = 0; i < num_of_consumers; i++)
    {
        pthread_join(consumers[i], NULL);
    }
    pthread_join(interruptor, NULL);

    // get result from random consumer
    return c_args[rand() % num_of_consumers].sum;
}

int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        return 1;
    }

    num_of_consumers = atoi(argv[1]);
    sleep_time = atoi(argv[2]);

    srand(time(NULL));

    std::cout << run_threads() << std::endl;
    return 0;
}
