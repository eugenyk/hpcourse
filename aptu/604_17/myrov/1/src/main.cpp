#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>

#include <pthread.h>

// I think there is simpler solution with concurrent Value class but let's assume that I can't change it.
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


enum class POSSIBLE_STATUS
{
    WAITING_CONSUMER     = 0,
    WAITING_PRODUCER     = 1,
    STOPPING             = 2
};

struct RAIILock
{
    RAIILock(pthread_mutex_t *mutex) : 
        m{mutex} 
    {
        pthread_mutex_lock(m);   
    }
    ~RAIILock()
    {
        pthread_mutex_unlock(m);
    }

private:
    pthread_mutex_t *m;
};

pthread_t consumer_thread; // we need to have global consumer thread variable to be able to interrupt it

pthread_mutex_t global_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

POSSIBLE_STATUS status = POSSIBLE_STATUS::WAITING_CONSUMER;

std::vector<int> read_data(std::istream &is)
{
    std::vector<int> res;

    std::copy(std::istream_iterator<int>(is),
              std::istream_iterator<int>(),
              std::back_inserter(res));

    return res;
}

void *producer_routine(void *arg) 
{    
    Value *v_ptr = reinterpret_cast<Value *>(arg);
    
    std::vector<int> input = read_data(std::cin);

    pthread_mutex_lock(&global_mutex);
    for(int v : input) 
    {
        if(status == POSSIBLE_STATUS::WAITING_CONSUMER) 
        {
            pthread_cond_wait(&cond, &global_mutex); // waiting consumer thread to handle previous number
        }

        v_ptr->update(v);
        status = POSSIBLE_STATUS::WAITING_CONSUMER;
    }

    status = POSSIBLE_STATUS::WAITING_CONSUMER;

    pthread_cond_wait(&cond, &global_mutex); // waiting consumer thread to handle last number

    status = POSSIBLE_STATUS::STOPPING; // we are done.
    pthread_mutex_unlock(&global_mutex); // final producer unlock

    return nullptr;
}

void *consumer_routine(void *arg) 
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr); // we don't want consumer thread to be cancelled

    int *res = new int(0);
    Value *val = reinterpret_cast<Value *>(arg);
    POSSIBLE_STATUS local_status = POSSIBLE_STATUS::WAITING_CONSUMER; // using local status variable require some work to update it but it is clearer


    while (local_status != POSSIBLE_STATUS::STOPPING) 
    {
        RAIILock lock(&global_mutex);

        if (status == POSSIBLE_STATUS::WAITING_CONSUMER) 
        {
            (*res) += val->get();

            status = POSSIBLE_STATUS::WAITING_PRODUCER;

            pthread_cond_broadcast(&cond); // notify interrupter and producer that consumer is ready
        }

        local_status = status;
    }

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr); // now this thread can be cancelled

    return res;
}

void *consumer_interruptor_routine(void *arg) 
{
    pthread_mutex_lock(&global_mutex);

    if(status == POSSIBLE_STATUS::WAITING_CONSUMER) 
    {
        pthread_cond_wait(&cond, &global_mutex); // waiting consumer thread to be 
    }

    pthread_mutex_unlock(&global_mutex);


    POSSIBLE_STATUS local_status = POSSIBLE_STATUS::WAITING_CONSUMER;
    while (local_status != POSSIBLE_STATUS::STOPPING) 
    {
        RAIILock lock(&global_mutex);

        pthread_cancel(consumer_thread); //trying to cancel consumer thread

        local_status = status;
    }

    return nullptr;
}

int run_threads() 
{
    Value val;
    int *res;

    pthread_t producer_thread, interruptor_thread;

    pthread_create(&producer_thread, nullptr, producer_routine, reinterpret_cast<void *>(&val));
    pthread_create(&consumer_thread, nullptr, consumer_routine, reinterpret_cast<void *>(&val));
    pthread_create(&interruptor_thread, nullptr, consumer_interruptor_routine, nullptr);

    pthread_join(producer_thread, nullptr);
    pthread_join(consumer_thread, reinterpret_cast<void **>(&res));
    pthread_join(interruptor_thread, nullptr);

    int sum = *res; // a bit messy but we don't want memory leak
    delete res;
    
    return sum;
}

int main(int argc, char **argv) 
{
    int res = run_threads();
    std::cout << res << std::endl;
    
    return 0;
}