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
  // Wait for consumer to start
 
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
}
 
void* consumer_routine(void* arg)
{
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
}
 
void* consumer_interruptor_routine(void* arg)
{
  // wait for consumer to start
 
  // interrupt consumer while producer is running                                          
}
 
int run_threads()
{
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
 
  return 0;
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

    std::cout << "consumers threads num: " << consumersNum << std::endl;
    std::cout << "sleepLimit: " << sleepLimit << std::endl;

    std::cout << run_threads() << std::endl;
    return 0;
}

