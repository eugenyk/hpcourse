#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <pthread.h>
#include <unistd.h>

// from argv
int num_consumers;
int sleep_time;

int num_consumers_started = 0;
int num_consumers_updated = 0;

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t all_consumers_started_cond = PTHREAD_COND_INITIALIZER;
bool consumers_started = false;

pthread_cond_t consumers_done_update_cond = PTHREAD_COND_INITIALIZER;
bool consumers_updated = false;

pthread_cond_t producer_updated_value_cond = PTHREAD_COND_INITIALIZER;
bool producer_updated = false;

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
    Value* value;
    std::vector<int> digits;
};

struct consumer_args
{
    consumer_args() : sum(0) { }
    consumer_args(Value* _value) : sum(0), value(_value) { }
    Value* value;
    int sum;
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
    for (it = args->digits.begin(); it != args->digits.end(); it++)
    {
        (args->value)->update(*it);
        producer_updated = true;
        consumers_updated = false;

        // notify consumers
        pthread_cond_broadcast(&producer_updated_value_cond);

        // wait for consumers to update sum
        while (!consumers_updated)
        {
            pthread_cond_wait(&consumers_done_update_cond, &value_mutex);
        }
    }

    pthread_mutex_unlock(&value_mutex);

    // notify about exit
    done = true;
    pthread_cond_broadcast(&producer_updated_value_cond);
}

void* consumer_routine(void* arg)
{
    consumer_args* args = (consumer_args*)arg;

    pthread_mutex_lock(&value_mutex);

    // count started consumers
    num_consumers_started++;
    if (num_consumers_started == num_consumers)
    {
        consumers_started = true;
        pthread_cond_signal(&all_consumers_started_cond);
    }

    for (;;)
    {
        // wait for producer
        //while (!producer_updated)
        //{
            pthread_cond_wait(&producer_updated_value_cond, &value_mutex);
        //}

        if (done)
        {
            break;
        }

        args->sum += (args->value)->get();

        // count consumers that updated sum
        num_consumers_updated++;
        if (num_consumers_updated == num_consumers)
        {
            num_consumers_updated = 0;
            consumers_updated = true;
            producer_updated = false;
            pthread_cond_signal(&consumers_done_update_cond);
        }

        // sleep for a random time
        int sleep_time_rnd = 0;
        if (sleep_time != 0)
        {
            srand(time(NULL));
            sleep_time_rnd = rand() % sleep_time;
        }
        usleep(sleep_time_rnd * 1000);
    }

    pthread_mutex_unlock(&value_mutex);
}

void* consumer_interruptor_routine(void* arg) {
  // wait for consumer to start

  // interrupt consumer while producer is running
}

int run_threads()
{
    // read digits from in stream
    std::vector<int> digits;
    std::string buffer;
    std::getline(std::cin, buffer);
    std::istringstream iss(buffer);
    int i;

    while (iss >> i)
    {
        digits.push_back(i);
    }

    // shared value
    Value* value = new Value();

    // create producer
    producer_args p_args;
    p_args.digits = digits;
    p_args.value = value;

    pthread_t producer;
    pthread_create(&producer, NULL, &producer_routine, &p_args);

    // create N consumers
    pthread_t* consumers = new pthread_t[num_consumers];
    consumer_args* c_args = new consumer_args[num_consumers];
    for (i = 0; i < num_consumers; i++)
    {
        c_args[i] = consumer_args(value);
        pthread_create(&consumers[i], NULL, &consumer_routine, &c_args[i]);
    }

    // wait for threads
    pthread_join(producer, NULL);
    for (i = 0; i < num_consumers; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    delete[] consumers;

    return c_args[0].sum;
}

int main(int argc, char* argv[])
{
    if (argc <= 2)
    {
        return 1;
    }

    num_consumers = atoi(argv[1]);
    sleep_time = atoi(argv[2]);

    std::cout << run_threads() << std::endl;
    return 0;
}
