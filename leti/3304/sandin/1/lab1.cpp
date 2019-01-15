#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int max_sleep_time;
int count_consumers;

int count_consumers_started = 0;
int count_consumers_updated = 0;
int count_producer_update = 0;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t cond_start_consumer = PTHREAD_COND_INITIALIZER;
bool consumers_start = false;

pthread_cond_t cond_update_consumer = PTHREAD_COND_INITIALIZER;
bool consumer_update = false;

pthread_cond_t cond_update_producer = PTHREAD_COND_INITIALIZER;

bool done = false;

class Value {
public:
    Value() : _value(0) { }
 
    void update(int value) {
        _value = value;
    }
 
    int get() const {
        return _value;
    }
 
private:
    int _value;
};
 
void* producer_routine(void* arg) {
	// Wait for consumer to start

    Value* shared_arg = (Value*) arg;
    pthread_mutex_lock(&lock);
    while(!consumers_start) 
	{
		pthread_cond_wait(&cond_start_consumer, &lock);
	}
	
    pthread_mutex_unlock(&lock);

	// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
	string nums;
    getline(cin, nums);
    stringstream inStream;
    inStream << nums;
	pthread_mutex_lock(&lock);
    while(!done)
    {
        int number;
        if (inStream >> number)
        {
            shared_arg->update(number);
            count_producer_update++;
            pthread_cond_broadcast(&cond_update_producer);
			consumer_update = false;
            while(!consumer_update) 
			{
				pthread_cond_wait(&cond_update_consumer, &lock);
			}
        } 
		else 
		{
            done = true;
        }
    }
	
	pthread_mutex_unlock(&lock);
    pthread_cond_broadcast(&cond_update_producer);
}
 
void* consumer_routine(void* arg) {
	// notify about start
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	Value* shared_arg = (Value*) arg;
	pthread_mutex_lock(&lock);

	count_consumers_started++;
	if (count_consumers_started == count_consumers)
	{
		consumers_start = true;
		pthread_cond_broadcast(&cond_start_consumer);
	}
	pthread_mutex_unlock(&lock);

	// for every update issued by producer, read the value and add to sum
	Value* result = new Value();
	int count_local_update = 0;

	while(!done)
	{
		 pthread_mutex_lock(&lock);

		 while(!done && (count_local_update == count_producer_update)) 
		 {
			pthread_cond_wait(&cond_update_producer, &lock);
		 }
		 
		 if (done)
		 {
			 break;
		 }
	 
		 result->update(result->get() + shared_arg->get());
		 count_consumers_updated++;
		 
		 count_local_update++;
		 
		 if (count_consumers_updated == count_consumers)
		 {
			 consumer_update = true;
			 count_consumers_updated = 0;
			 pthread_cond_signal(&cond_update_consumer);
		 }
		 pthread_mutex_unlock(&lock);

		 usleep(rand() % (max_sleep_time * 1000 + 1));
	}
	
	pthread_mutex_unlock(&lock);	 
    // return pointer to result (aggregated result for all consumers)
    return (void*) result;
}
 
void* consumer_interruptor_routine(void* arg) {
	// wait for consumer to start
    pthread_t* threads = (pthread_t*) arg;
    pthread_mutex_lock(&lock);
    while(!consumers_start) 
	{
		pthread_cond_wait(&cond_start_consumer, &lock);
	}
	
    pthread_mutex_unlock(&lock);
	
	// interrupt consumer while producer is running
    while(!done)
    {
        size_t random_id = rand() % count_consumers;
        pthread_cancel(threads[random_id]);
    }
}
 
int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values

	Value* value = new Value();
    pthread_t producer;
	pthread_t consumers[count_consumers];
	pthread_t interruptor;
	void* res;
	
    pthread_create(&producer, 0, producer_routine, value);

	for (int i = 0; i < count_consumers; i++)
	{
        pthread_create(&consumers[i], 0, consumer_routine, value);
	}
	
	pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumers);
	
	pthread_join(producer, NULL);
    pthread_join(interruptor, NULL);
	pthread_join(consumers[0], &res);
	
    for (int i = 1; i < count_consumers; i++)
    {
        pthread_join(consumers[i], NULL);
    }
	
	return *(int *) res;
}
 
int main(int argc, const char *argv[]) {
    if (argc != 3)
    {
        cout << "Incorrect parameters!" << endl;
        return 1;
    }

    count_consumers = atoi(argv[1]);
    max_sleep_time = atoi(argv[2]);
	  
	srand(time(NULL));
	
    cout << run_threads() << endl;

	return 0;
}
