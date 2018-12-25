#include <pthread.h>
#include <string>
#include <iostream>
#include <vector>
#include <sstream>
#include <ctime>
#include <unistd.h>
#include <cstdlib>

using namespace std;

int maxTimeOfSleep;
int numberOfConsumers;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

pthread_cond_t condOfStartC = PTHREAD_COND_INITIALIZER;
bool isStartOfConsumers = false;

pthread_cond_t condOfUpdateC = PTHREAD_COND_INITIALIZER;
bool isUpdateOfConsumers = false;

pthread_cond_t condOfUpdatePr = PTHREAD_COND_INITIALIZER;

bool complete = false;

int numberOfStartedConsuners = 0;
int numberOfUpdatedConsumers = 0;
int numberOfProducerUpdate = 0;

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
    while(!isStartOfConsumers) 
	{
		pthread_cond_wait(&condOfStartC, &lock);
	}

    pthread_mutex_unlock(&lock);

	// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
	string numbers;
    getline(cin, numbers);
    stringstream inStream;
    inStream << numbers;

    while(!complete)
    {
        int number;
        if (inStream >> number)
        {
            shared_arg->update(number);
            numberOfProducerUpdate++;
            pthread_cond_broadcast(&condOfUpdatePr);
            pthread_mutex_lock(&lock);
			isUpdateOfConsumers = false;
            while(!isUpdateOfConsumers) 
			{
				pthread_cond_wait(&condOfUpdateC, &lock);
			}
            pthread_mutex_unlock(&lock);
        } 
		else 
		{
            complete = true;
        }
    }

    pthread_cond_broadcast(&condOfUpdatePr);
}

void* consumer_routine(void* arg) {
	// notify about start
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	Value* shared_arg = (Value*) arg;
	pthread_mutex_lock(&lock);

	numberOfStartedConsuners++;
	if (numberOfStartedConsuners == numberOfConsumers)
	{
		isStartOfConsumers = true;
		pthread_cond_broadcast(&condOfStartC);
	}
	pthread_mutex_unlock(&lock);

	// for every update issued by producer, read the value and add to sum
	Value* result = new Value();
	int count_local_update = 0;

	while(!complete)
	{
		 pthread_mutex_lock(&lock);

		 while(!complete && (count_local_update == numberOfProducerUpdate)) 
		 {
			pthread_cond_wait(&condOfUpdatePr, &lock);
		 }
		 pthread_mutex_unlock(&lock);

		 if (complete)
		 {
			 break;
		 }

		 result -> update(result -> get() + shared_arg -> get());
		 pthread_mutex_lock(&lock);
		 numberOfUpdatedConsumers++;
		 pthread_mutex_unlock(&lock);
		 count_local_update++;

		 if (numberOfUpdatedConsumers == numberOfConsumers)
		 {
			 isUpdateOfConsumers = true;
			 numberOfUpdatedConsumers = 0;
			 pthread_cond_signal(&condOfUpdateC);
		 }

		 usleep(rand() % (maxTimeOfSleep * 1000 + 1));
	}

    // return pointer to result (aggregated result for all consumers)
    return (void*) result;
}

void* consumer_interruptor_routine(void* arg) {
	// wait for consumer to start
    pthread_t* threads = (pthread_t*) arg;
    pthread_mutex_lock(&lock);
    while(!isStartOfConsumers) 
	{
		pthread_cond_wait(&condOfStartC, &lock);
	}

    pthread_mutex_unlock(&lock);

	// interrupt consumer while producer is running
    while(!complete)
    {
        size_t random_id = rand() % numberOfConsumers;
        pthread_cancel(threads[random_id]);
    }
}

int run_threads() {
  // start N threads and wait until they're done
  // return aggregated sum of values

	Value* value = new Value();
    pthread_t producer;
	pthread_t consumers[numberOfConsumers];
	pthread_t interruptor;
	void* res;

    pthread_create(&producer, 0, producer_routine, value);

	for (int i = 0; i < numberOfConsumers; i++)
	{
        pthread_create(&consumers[i], 0, consumer_routine, value);
	}

	pthread_create(&interruptor, NULL, consumer_interruptor_routine, &consumers);

	pthread_join(producer, NULL);
    	pthread_join(interruptor, NULL);
	pthread_join(consumers[0], &res);

    for (int i = 1; i < numberOfConsumers; i++)
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

    numberOfConsumers = atoi(argv[1]);
    maxTimeOfSleep = atoi(argv[2]);

	srand(time(NULL));

    cout << run_threads() << endl;

	return 0;
}
