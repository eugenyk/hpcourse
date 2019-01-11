#include <iostream>
#include <pthread.h>
#include <vector>
#include <sstream>
#include <string>

int consumersCount;
int sleepLimit;

// Consumers start condition
pthread_cond_t consTCond = PTHREAD_COND_INITIALIZER;
volatile bool consTReady = false;

// Value update sync
pthread_mutex_t valMutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t valCond = PTHREAD_COND_INITIALIZER;
volatile bool valReady = false;
volatile bool inputDone = false;

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
 
void* producer_routine(void* arg)
{
    Value *value = reinterpret_cast<Value *>(arg);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    std::vector<int> numbers;

    pthread_mutex_lock(&valMutex);
    while (!consTReady)
    {
        pthread_cond_wait(&consTCond, &valMutex);
    }
    pthread_mutex_unlock(&valMutex);

    std::string buf;
    std::getline(std::cin, buf);
    std::istringstream ss(buf);

    int inputNumber;
    while (ss >> inputNumber)
    {
        numbers.push_back(inputNumber);
    }

    for (auto number: numbers)
    {
        // consumers are ready, update the value
        pthread_mutex_lock(&valMutex);
        value->update(number);
        valReady = true;
        pthread_cond_broadcast(&valCond);

        // Wait for consumer to start
        do
        {
            pthread_cond_wait(&consTCond, &valMutex);
        }
        while (!consTReady);

        consTReady = false;
        valReady = false;
        pthread_mutex_unlock(&valMutex);
    }

    pthread_mutex_lock(&valMutex);
    inputDone = true;
    pthread_cond_broadcast(&valCond);
    valReady = true;
    pthread_mutex_unlock(&valMutex);
    
    pthread_exit(nullptr);
}
 
void* consumer_routine(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    static int count = 0;
    Value *value = reinterpret_cast<Value *>(arg);

    // allocate value for result
    Value *result = new Value();

    while (1)
    {
        pthread_mutex_lock(&valMutex);
        count ++;
        if (count >= consumersCount)
        {
            valReady = false;
            
            // notify about start
            consTReady = true;
            
            pthread_cond_broadcast(&consTCond);

            count = 0;
        }

        do
        {
            ;pthread_cond_wait(&valCond, &valMutex);
        }
        while (!valReady);

        consTReady = false;

        if (!inputDone)
        {
            // for every update issued by producer, read the value and add to sum
            result->update(value->get() + result->get());
        }
        else
        {
            pthread_mutex_unlock(&valMutex);

            // return pointer to result            
            pthread_exit(reinterpret_cast<void *>(result));
        }

        pthread_mutex_unlock(&valMutex);

        timespec ts;
        ts.tv_sec = sleepLimit == 0 ? 0 : (float)(rand() % sleepLimit) / 1000.0;
        ts.tv_nsec = 0;
        nanosleep(&ts, nullptr);
    }
}
 
void* consumer_interruptor_routine(void* arg)
{
    // wait for consumer to start
    pthread_t *consumers = reinterpret_cast<pthread_t *>(arg);
    
    // interrupt consumer while producer is running   
    while (!inputDone)
    {
        unsigned i = rand() % consumersCount;
        pthread_cancel(consumers[i]);
    }

    pthread_exit(nullptr);
}
 
int run_threads(int consumersNum)
{
    // start 3 threads and wait until they're done
    Value *value = new Value();
    Value *result;

    consumersCount = consumersNum;

    // Threads generation
    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value);
    
    pthread_t *consumers = new pthread_t[consumersNum];
    for (size_t i = 0; i < consumersNum; i++)
    {
        pthread_create(&consumers[i], nullptr, consumer_routine, value);
    }
    
    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, consumers);

    // Join consumers
    for (size_t i = 0; i < consumersNum; i++)
    {
        pthread_join(consumers[i], reinterpret_cast<void **>(&result));
    }

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    delete[] consumers;

    // return sum of update values seen by consumer

    return result->get();
}
 
int main(int argc, const char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Usage:\n\t1st argument - number of consumer-threads\n\t2nd argument - sleep limit in ms\n";
        return 1;
    }

    int consumersNum = atoi(argv[1]);
    sleepLimit = atoi(argv[2]);

    std::cout << run_threads(consumersNum) << std::endl;
    return 0;
}