#include <pthread.h>
#include <iostream>
#include <stdexcept>

bool consumer_is_waiting = false;
bool consumer_started = false;
bool all_data_handled = false;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t consumer_ready_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t data_ready_cond = PTHREAD_COND_INITIALIZER;

class Value 
{
public:
    Value() 
        : value_(0) 
    { }

    void update(int value) 
    {
        value_ = value;
    }

    int get() const 
    {
        return value_;
    } 

private:
    int value_;
};

void * producer_routine(void * args) 
{
    Value * val = (Value *) args;
    int next_value;
    while (std::cin >> next_value) 
    {
        pthread_mutex_lock(&mutex);
        while (!consumer_is_waiting) 
        {
            pthread_cond_wait(&consumer_ready_cond, &mutex); 
        }
        val->update(next_value);
        consumer_is_waiting = false;
        pthread_cond_signal(&data_ready_cond); 
        pthread_mutex_unlock(&mutex);
    }

    pthread_mutex_lock(&mutex);
    all_data_handled = true;
    consumer_is_waiting = false;
    pthread_cond_signal(&data_ready_cond); 
    pthread_mutex_unlock(&mutex);
}

void * consumer_routine(void * args) 
{
    Value * val = (Value *) args;
    int * total = new int(0);

    int s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    if (s != 0) {
        throw std::runtime_error("consumer cannot set cancellation state");
    }

    while (!all_data_handled) 
    {
        pthread_mutex_lock(&mutex);

        if (all_data_handled) 
        {
            pthread_mutex_unlock(&mutex);
            break;
        }

        consumer_is_waiting = true;
        consumer_started = true;
        pthread_cond_signal(&consumer_ready_cond); 

        while (consumer_is_waiting) 
        {
            pthread_cond_wait(&data_ready_cond, &mutex); 
        }
        if (!all_data_handled) 
        {
            *total += val->get();
        }
        pthread_mutex_unlock(&mutex);
    }

    pthread_exit(total);
}

void * consumer_interruptor_routine(void * args) 
{
    pthread_t * to_interrupt = (pthread_t *) args;

    pthread_mutex_lock(&mutex);
    while (!consumer_started) 
    {
        pthread_cond_wait(&consumer_ready_cond, &mutex); 
    }
    pthread_mutex_unlock(&mutex);

    while (true) 
    {
        pthread_mutex_lock(&mutex);
        if (all_data_handled) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        else {
            pthread_cancel(*to_interrupt);
        }
        pthread_mutex_unlock(&mutex);
    }                                    
}

int run_threads() 
{
    Value * val = new Value();

    pthread_t producer;
    pthread_t consumer;
    pthread_t interruptor;
 
    int producer_status = pthread_create(&producer, NULL, producer_routine, val);
    int consumer_status = pthread_create(&consumer, NULL, consumer_routine, val);
    int interruptor_status = pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumer);

    if (producer_status != 0 || consumer_status != 0 || interruptor_status != 0) {
        throw std::runtime_error(":(");
    }

    int * res;
    pthread_join(producer, NULL);
    pthread_join(consumer, (void * *)&res);
    pthread_join(interruptor, NULL);

    int toReturn = *res;

    delete res;
    delete val;

    return toReturn;
}

int main() 
{
    std::cout << run_threads() << std::endl;

    return 0;
}