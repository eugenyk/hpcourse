#include <pthread.h>

#include <iostream>
 
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

enum status{START, WAIT_FOR_CONSUMMER, WAIT_FOR_PRODUCER, STOP};

status current_status = START;

pthread_t producer;
pthread_t consumer;
pthread_t interruptor;

pthread_mutex_t mutex;

pthread_cond_t consumer_start_cond;
pthread_cond_t consumer_wait_cond;

// Wait for consumer to start
void wait_for_consumer_to_start()
{
	pthread_mutex_lock(&mutex);
	while(current_status == START)
	{
		pthread_cond_wait(&consumer_start_cond, &mutex);
	}
	pthread_mutex_unlock(&mutex);
}

 
void* producer_routine(void* arg) {

	wait_for_consumer_to_start();
	
	//cast arg
	Value *val = reinterpret_cast<Value *>(arg);
 
	// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
	int input_value;
	while (std::cin >> input_value)
	{
		pthread_mutex_lock(&mutex);
		//Wait for consumer to process
		while(current_status == WAIT_FOR_CONSUMMER)
		{
			pthread_cond_wait(&consumer_wait_cond, &mutex);
		}

		val->update(input_value);

		//Notify consumer
		current_status = WAIT_FOR_CONSUMMER;
		
		pthread_mutex_unlock(&mutex);
	}

	//Wait for consumer processing last number
	pthread_mutex_lock(&mutex);
	while(current_status == WAIT_FOR_CONSUMMER)
	{
		pthread_cond_wait(&consumer_wait_cond, &mutex);
	}
	current_status = STOP;
	pthread_mutex_unlock(&mutex);
}
 
void* consumer_routine(void* arg) {
	// protect consumer from interruptor
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr); 

	//cast arg
	Value *val = reinterpret_cast<Value *>(arg);
	
	// notify about start
	pthread_mutex_lock(&mutex);
	current_status = WAIT_FOR_PRODUCER;
	//The pthread_cond_broadcast() routine should be used instead of pthread_cond_signal() if more than one thread is in a blocking wait state.
	pthread_cond_broadcast(&consumer_start_cond);
	pthread_mutex_unlock(&mutex);

	// allocate value for result
	int *result_sum = new int(0);

	// for every update issued by producer, read the value and add to sum
	while(true)
	{
		pthread_mutex_lock(&mutex);
		if (current_status == STOP)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}
		if (current_status == WAIT_FOR_CONSUMMER)
		{
			*result_sum += val->get();
			current_status = WAIT_FOR_PRODUCER;
			pthread_cond_broadcast(&consumer_wait_cond);
		}
		pthread_mutex_unlock(&mutex);
	}

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
	
	// return pointer to result
	return result_sum;
}
 
void* consumer_interruptor_routine(void* arg) {
	// wait for consumer to start
	wait_for_consumer_to_start();

	// interrupt consumer while producer is running   
	while(true)
	{
		pthread_mutex_lock(&mutex);
		if (current_status == STOP)
		{
			pthread_mutex_unlock(&mutex);
			break;
		}

		pthread_cancel(consumer);
		pthread_mutex_unlock(&mutex);
	}
}
 
int run_threads() {
	// start 3 threads and wait until they're done
	Value* val = new Value();

	pthread_create(&producer, nullptr, producer_routine, (void*) val);
	pthread_create(&consumer, nullptr, consumer_routine, (void*) val);
	pthread_create(&interruptor, nullptr, consumer_interruptor_routine, (void*) val);

	void * consumer_result = nullptr;
	pthread_join(producer, nullptr);
	pthread_join(consumer, &consumer_result);
	pthread_join(interruptor, nullptr);

	// return sum of update values seen by consumer
	int result_sum = * (int*) consumer_result;

	delete (int*) consumer_result;
	delete val;
 
	return result_sum;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}
