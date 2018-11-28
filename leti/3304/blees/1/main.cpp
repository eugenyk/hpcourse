#include <iostream>
#include <pthread.h>
 
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
    printf("\t PR val = %d\n", value->get());
    
    // Wait for consumer to start
 
    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    pthread_exit(nullptr);
}
 
void* consumer_routine(void* arg)
{
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum

    Value *value = reinterpret_cast<Value *>(arg);
    printf("\t CS val = %d\n", value->get());
    Value *result = new Value();
    result->update(0);
    
    // return pointer to result
    pthread_exit(reinterpret_cast<void *>(result));
}
 
void* consumer_interruptor_routine(void* arg)
{
    // wait for consumer to start
    
    // interrupt consumer while producer is running     
    
    const unsigned sleepLimit = *reinterpret_cast<const unsigned *>(arg);
    printf("\t IN sleep = %d\n", sleepLimit);
    pthread_exit(nullptr);
}
 
int run_threads(int consumersNum, int sleepLimit)
{
    // start 3 threads and wait until they're done
    Value *value = new Value();
    Value *result;

    // Threads generation
    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value);
    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &sleepLimit);
    
    pthread_t *consumers = new pthread_t[consumersNum];
    for (size_t i = 0; i < consumersNum; i++)
    {
        pthread_create(&consumers[i], nullptr, consumer_routine, value);
    }

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
    int sleepLimit = atoi(argv[2]);

    std::cout << run_threads(consumersNum, sleepLimit) << std::endl;
    return 0;
}