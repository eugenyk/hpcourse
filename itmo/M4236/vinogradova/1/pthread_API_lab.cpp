#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <cstdlib>
#include <string>
#include <sstream>

using namespace std;

class Value
{
public:
    Value() : _value(0)
    { }
 
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

int sum = 0;

bool next_value = false;
bool end_of_producer = false;
bool start_consumer = false;

pthread_mutex_t value_mutex;
pthread_mutex_t start_consumer_mutex;

pthread_cond_t next_value_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t got_value_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t start_consumer_cond = PTHREAD_COND_INITIALIZER;

size_t MAX_TIME_SLEEP;
size_t N; // consumers number


// Wait for consumer to start
// Read data, loop through each value and update the value, 
// notify consumer, wait for consumer to process
void* producer_routine(void* arg)
{
    Value* shared_value = (Value*)(arg);

    pthread_mutex_lock(&start_consumer_mutex);
    while(!start_consumer)
        pthread_cond_wait(&start_consumer_cond, &start_consumer_mutex);
    pthread_mutex_unlock(&start_consumer_mutex);

    string str;
    getline(cin, str);
    stringstream ss(str);
    int read_value;
    int test_sum = 0;
    while (ss >> read_value)
    {
        test_sum += read_value;
        pthread_mutex_lock(&value_mutex);
        next_value = true;
        shared_value->update(read_value);
        pthread_cond_signal(&next_value_cond);
        pthread_mutex_unlock(&value_mutex);

        pthread_mutex_lock(&value_mutex);
        while (next_value)
            pthread_cond_wait(&got_value_cond, &value_mutex);
        pthread_mutex_unlock(&value_mutex);
    }

    pthread_mutex_lock(&value_mutex);
    end_of_producer = true;
    pthread_cond_broadcast(&next_value_cond);
    pthread_mutex_unlock(&value_mutex);
    std::cout << "Test value = " << test_sum << std::endl;
}

// notify about start
// for every update issued by producer, read the value and add to sum
// return pointer to result (aggregated result for all consumers)
void* consumer_routine(void* arg)
{
    int rc = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    
    Value* shared_value = (Value*)(arg);
    
    pthread_mutex_lock(&start_consumer_mutex);
    if (!start_consumer)
    {
        start_consumer = true;
        pthread_cond_broadcast(&start_consumer_cond);
    }
    pthread_mutex_unlock(&start_consumer_mutex);

    while (!end_of_producer)
    {
        pthread_mutex_lock(&value_mutex);
        while (!next_value)
        {
            if (!end_of_producer)
                pthread_cond_wait(&next_value_cond, &value_mutex);
            else
                break;
        }
        
        if (next_value)
        {
            sum += shared_value->get();
            next_value = false;
            pthread_cond_signal(&got_value_cond);
        }

        pthread_mutex_unlock(&value_mutex);

        int time_sleep = rand() % (MAX_TIME_SLEEP + 1);
        usleep(time_sleep);
    }

    return &sum;
}

// wait for consumer to start
// interrupt consumer while producer is running
void* consumer_interruptor_routine(void* arg)
{
    pthread_t* threads = (pthread_t*) arg;

    pthread_mutex_lock(&start_consumer_mutex);
    while(!start_consumer)
        pthread_cond_wait(&start_consumer_cond, &start_consumer_mutex);
    pthread_mutex_unlock(&start_consumer_mutex);
    
    while (!end_of_producer)
    {
        size_t random_thread = rand() % N;
        pthread_cancel(threads[random_thread]);
    }
}

int run_threads()
{
    pthread_t consumers[N];
    pthread_t producer;
    pthread_t interruptor;
    Value shared_value;


    int rc = pthread_create(&producer, NULL, producer_routine, (void *) &shared_value);

    for(size_t i = 0; i < N; i++) 
    {
      rc = pthread_create(&consumers[i], NULL, consumer_routine, (void *) &shared_value);
      if (rc)
         cout << "Error:unable to create thread," << rc << endl;
    }

    rc = pthread_create(&interruptor, NULL, consumer_interruptor_routine, (void *) &consumers);
    
    void* result;
    for (size_t i = 0; i < N; i++)
    {
        rc = pthread_join(consumers[i], &result);
        if (rc != 0)
        {
            cout << "Joing thread " << i << " failed!" << endl;
            return 1;
        }
    }

    rc = pthread_join(interruptor, NULL);
    rc = pthread_join(producer, NULL);

    return *((int *)result);
}
 
int main(int argc, char* argv[]) 
{
    if (argc < 3)
    {
        std::cout << "You should have count of consumers and sleep time as parameters" << std::endl;
        return 0;
    }
    else
    {
        N = strtol(argv[1], NULL, 10);
        MAX_TIME_SLEEP = strtol(argv[2], NULL, 10);
        std::cout << run_threads() << std::endl;
    }
    return 0;
}