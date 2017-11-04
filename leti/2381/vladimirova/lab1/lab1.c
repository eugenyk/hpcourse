#include <pthread.h>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

using namespace std;

int is_started = 0;
int can_read = 0;
int is_finished = 0;

pthread_mutex_t t_mutex;
pthread_cond_t t_cond;

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

Value *value = new Value();

struct arguments_array {
	int length;
	int* elements;
};

void* producer_routine(void* arg) {
	
	cout << "PRODUCER_ROUTINE: started " << endl;

	pthread_mutex_lock(&t_mutex);
	// cout << "PRODUCER_ROUTINE: waiting for consumer to start... (cond var)" << endl;
	if (!is_started) {
		cout << "PRODUCER_ROUTINE: waiting for consumer to start... (cond var)" << endl;
		pthread_cond_wait(&t_cond, &t_mutex);
	}
	pthread_mutex_unlock(&t_mutex);
	cout << "PRODUCER_ROUTINE: continue working" << endl;

	arguments_array* user_arguments = (arguments_array*) arg;

	for(int i = 0; i < user_arguments->length; i++) {
		pthread_mutex_lock(&t_mutex);
		
		if (can_read)
			pthread_cond_wait(&t_cond, &t_mutex);

		// cout << "input elements: " << user_arguments->elements[i] << endl;
		value->update(user_arguments->elements[i]);
		cout << "PRODUCER_ROUTINE: wrote " << value->get() << endl;
		can_read = 1;
		pthread_cond_signal(&t_cond);
		
		pthread_mutex_unlock(&t_mutex);
	}

	is_finished = 1;
	cout << "PRODUCER_ROUTINE: finished" << endl;
	
	pthread_exit(0);
	
}

void* consumer_routine(void* arg) {
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    cout << "CONSUMER_ROUTINE: started" << endl;
    int sum = 0;
    // notify about start
	pthread_mutex_lock(&t_mutex);
	is_started = 1;
	pthread_cond_signal(&t_cond);
	pthread_mutex_unlock(&t_mutex);

	for (;;) { 
		pthread_mutex_lock(&t_mutex);
		if (!can_read) {
			cout << "CONSUMER_ROUTINE: wait for can_read" << endl;
			pthread_cond_wait(&t_cond, &t_mutex);
		}

		sum += value->get();
		cout << "CONSUMER_ROUTINE: SUM = " << sum << endl;

		can_read = 0;
		pthread_cond_signal(&t_cond);
        pthread_mutex_unlock(&t_mutex);
        
		if (is_finished) break;
	}

	pthread_exit((void*) sum);
}

void* consumer_interruptor_routine(void* arg) {
	pthread_t* interupted_thread = (pthread_t*) arg;
    // wait for consumer to start
	pthread_mutex_lock(&t_mutex);
	if (!is_started) {
		pthread_cond_wait(&t_cond, &t_mutex);
	}
	pthread_mutex_unlock(&t_mutex);

	while (!is_finished) {
        // interrupt consumer while producer is running   
		pthread_cancel(*interupted_thread);
	    //	cout << "Iterruptor: trying to shut down consumer" << endl;
    }
  	pthread_exit(NULL);                                     
}

int run_threads(arguments_array input_data) {
	int sum;

	pthread_t producer_thread, consumer_thread, interrupter_thread;
	pthread_create(&producer_thread, NULL, producer_routine, &input_data);
	pthread_create(&consumer_thread, NULL, consumer_routine, NULL);
	pthread_create(&interrupter_thread, NULL, consumer_interruptor_routine, &consumer_thread);

	pthread_join(producer_thread, NULL); 
	pthread_join(consumer_thread, (void **) &sum); 
	pthread_join(interrupter_thread, NULL); 

  	return sum;
}

int main(int argc, char *argv[]){

	cout << "Program started" << endl;

	if(argc > 1) {
		pthread_mutex_init(&t_mutex, NULL);
		pthread_cond_init(&t_cond, NULL);
	
		arguments_array user_arguments;
		user_arguments.length = argc - 1; 
		user_arguments.elements = (int*)malloc((user_arguments.length)*sizeof(int));
		for(int i = 0; i < user_arguments.length ; i++){
			user_arguments.elements[i] = atoi(argv[i+1]);
			// cout<< user_arguments.elements[i]<<endl;
		}
	
		 cout << run_threads(user_arguments) << endl;
	} 
	else {
		cout << "You didn't pass any arguments to program!" << endl;
	} 
	
    return 0;
}