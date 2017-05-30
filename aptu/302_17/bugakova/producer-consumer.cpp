
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
 
pthread_mutex_t condition_value_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condition_value_cond = PTHREAD_COND_INITIALIZER;

enum class State {NOT_STARTED, EMPTY, FULL, TERMINATED};
State state = State::NOT_STARTED;

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
 
void wait_for_start() {
	pthread_mutex_lock(&condition_value_mutex);
	while (state == State::NOT_STARTED) {
		pthread_cond_wait(&condition_value_cond, &condition_value_mutex);
	}
	pthread_mutex_unlock(&condition_value_mutex);
}

void* producer_routine(void* arg) {
  // Wait for consumer to start
	Value* value = (Value*) arg; 

	wait_for_start();
 	
  // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
	int data;
	while (scanf("%d ", &data) == 1) {
	 	pthread_mutex_lock(&condition_value_mutex);
	 	value->update(data);
	 	state = State::FULL;
	 	pthread_cond_signal(&condition_value_cond);
	 	while (state == State::FULL) {
	 		pthread_cond_wait(&condition_value_cond, &condition_value_mutex);
	 	}
	 	pthread_mutex_unlock(&condition_value_mutex);
	} 

	pthread_mutex_lock(&condition_value_mutex);
	state = State::TERMINATED;
	pthread_cond_signal(&condition_value_cond);
	pthread_mutex_unlock(&condition_value_mutex); 
}
 
void* consumer_routine(void* arg) {
	Value* value = (Value*) arg;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
  // notify about start
	pthread_mutex_lock(&condition_value_mutex);
	state = State::EMPTY;
	pthread_cond_broadcast(&condition_value_cond);
	pthread_mutex_unlock(&condition_value_mutex);
  // allocate value for result
	int* sum = new int;
  // for every update issued by producer, read the value and add to sum
	while (true) {
		pthread_mutex_lock(&condition_value_mutex);
		if (state == State::TERMINATED) {
			break;
		}
		while (state == State::EMPTY) {
			pthread_cond_wait(&condition_value_cond, &condition_value_mutex);
		}
		if (state == State::TERMINATED) 
			break;
		(*sum) += value->get();
		state = State::EMPTY;
		pthread_cond_signal(&condition_value_cond);
		pthread_mutex_unlock(&condition_value_mutex);
	}
  // return pointer to result
	return sum;
}
 
void* consumer_interruptor_routine(void* arg) {
	pthread_t cons = (pthread_t) arg;
  // wait for consumer to start
 	wait_for_start();
  // interrupt consumer while producer is running 
  while (state != State::TERMINATED) {
  	int rc = pthread_cancel(cons);
 	if (rc == 0) {
 		std::cout << "success!" << std::endl;
 		break;
 	}
  }                                         
}
 
int run_threads() {

	pthread_t cons, prod, inter;
	int iret_cons, iter_prod, iter_inter;

	Value * value = new Value();

	iter_prod = pthread_create(&prod, NULL, producer_routine, (void*)value);
	iter_inter = pthread_create(&inter, NULL, consumer_interruptor_routine, (void*)cons);
	iret_cons = pthread_create(&cons, NULL, consumer_routine, (void*)value);

  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
	void* returned;

 	pthread_join(cons, &returned);
 	pthread_join(prod, NULL);
 	pthread_join(inter, NULL);

 	int ans = *((int*) returned);

 	free(value);
 	free(returned);

	return ans;
}
 
int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}