#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <pthread.h>
#include <unistd.h>

// from argv
int num_consumers;

int num_consumers_started = 0;
int num_consumers_updated = 0;

pthread_mutex_t value_mutex = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t all_consumers_started_cond = PTHREAD_COND_INITIALIZER;
bool consumers_started = false;

pthread_cond_t consumers_done_update_cond = PTHREAD_COND_INITIALIZER;
bool consumers_updated = false;

pthread_cond_t producer_updated_value_cond = PTHREAD_COND_INITIALIZER;

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
    std::vector<int> digits;
    Value* value;
};

void* producer_routine(void* arg)
{
    pthread_mutex_lock(&value_mutex);

    // wait for consumers to start
    while (!consumers_started)
    {
        pthread_cond_wait(&all_consumers_started_cond, &value_mutex);
    }

    producer_args* args = (producer_args*)arg;

    std::vector<int>:: iterator it;
    for (it = args->digits.begin(); it != args->digits.end(); it++)
    {
        (args->value)->update(*it);
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
    Value* value = (Value*)arg;
    int sum = 0;

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
        pthread_cond_wait(&producer_updated_value_cond, &value_mutex);

        if (done)
        {
            break;
        }

        sum += value->get();

        // count consumers that updated sum
        num_consumers_updated++;
        if (num_consumers_updated == num_consumers)
        {
            num_consumers_updated = 0;
            consumers_updated = true;
            pthread_cond_signal(&consumers_done_update_cond);
        }
    }

    pthread_mutex_unlock(&value_mutex);

    std::cout << sum << std::endl;
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
    producer_args args;
    args.digits = digits;
    args.value = value;

    pthread_t producer;
    pthread_create(&producer, NULL, &producer_routine, &args);

    // create N consumers
    pthread_t* consumers = new pthread_t[num_consumers];
    for (i = 0; i < num_consumers; i++)
    {
        pthread_create(&consumers[i], NULL, &consumer_routine, value);
    }

    // wait for threads
    pthread_join(producer, NULL);
    for (i = 0; i < num_consumers; i++)
    {
        pthread_join(consumers[i], NULL);
    }

    delete[] consumers;

    return value->get();
}

int main(int argc, char* argv[])
{
    if (argc <= 1)
    {
        return 1;
    }

    num_consumers = atoi(argv[1]);

    std::cout << run_threads() << std::endl;
    return 0;
}
