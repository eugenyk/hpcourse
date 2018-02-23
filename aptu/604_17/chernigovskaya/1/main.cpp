#include <iostream>
#include <pthread.h>

class Value
{
public:
    Value() : _value(0)
    {}

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

pthread_t producer, consumer, interruptor;
pthread_mutex_t mutex;
pthread_cond_t cond;

enum State
{
    NOT_STARTED,
    PRODUCER_IS_RUNNING,
    CONSUMER_IS_RUNNING,
    STOPPED
};

State state = NOT_STARTED;


void wait_for_consumer_to_start()
{
    pthread_mutex_lock(&mutex);
    while (state == NOT_STARTED)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    pthread_mutex_unlock(&mutex);
}

void notify_about_start()
{
    pthread_mutex_lock(&mutex);
    state = CONSUMER_IS_RUNNING;
    pthread_cond_broadcast(&cond);
    pthread_mutex_unlock(&mutex);
}

void* producer_routine(void* arg)
{
    wait_for_consumer_to_start();

    pthread_mutex_lock(&mutex);
    int number;

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process   
    while (std::cin >> number)
    //for (number = 0; number < 10000; ++number)
    {
        while (state == CONSUMER_IS_RUNNING)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        reinterpret_cast<Value*>(arg)->update(number);
        state = CONSUMER_IS_RUNNING; 
    }
    while (state == CONSUMER_IS_RUNNING)
    {
        pthread_cond_wait(&cond, &mutex);
    }
    state = STOPPED;
    pthread_mutex_unlock(&mutex);
}

void* consumer_routine(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

    notify_about_start();

    // allocate value for result
    int *sum = new int;
    *sum = 0;

    // for every update issued by producer, read the value and add to sum
    while (state != STOPPED)
    {
        pthread_mutex_lock(&mutex);
        if (state == CONSUMER_IS_RUNNING)
        {
            *sum += reinterpret_cast<Value*>(arg)->get();
            state = PRODUCER_IS_RUNNING;
            pthread_cond_broadcast(&cond);
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    return sum;
}


void* consumer_interruptor_routine(void* arg)
{
    wait_for_consumer_to_start();

    // interrupt consumer while producer is running
    while (state != STOPPED)
    {
        pthread_mutex_lock(&mutex);
        pthread_cancel(consumer);
        pthread_mutex_unlock(&mutex);
    }
}


int run_threads()
{
    // start 3 threads and wait until they're done
    Value value;
    int* sum;
    pthread_create(&producer, NULL, producer_routine, reinterpret_cast<void*>(&value));
    pthread_create(&consumer, NULL, consumer_routine, reinterpret_cast<void*>(&value));
    pthread_create(&interruptor, NULL, consumer_interruptor_routine, NULL);

    pthread_join(producer, NULL);
    pthread_join(consumer, reinterpret_cast<void**>(&sum));
    pthread_join(interruptor, NULL);

    int result = *sum;
    delete sum;

    return result;
}

int main()
{
    std::cout << run_threads() << std::endl;
    return 0;
}