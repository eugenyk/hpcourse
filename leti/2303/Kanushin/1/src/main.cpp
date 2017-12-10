#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>

#include <iostream>

pthread_t prod_t;
pthread_t cons_t;
pthread_t inter_t;

pthread_cond_t consumer_ready;
pthread_cond_t value_updated;
pthread_cond_t value_read;
pthread_mutex_t value_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t cond_lock = PTHREAD_MUTEX_INITIALIZER;

bool consumer_ready_cond = false;
bool value_updated_cond = false;
bool value_read_cond = false;

class Value {
public:
	Value() :
			_value(0) {
	}

	void update(int value) {
		_value = value;
	}

	int get() const {
		return _value;
	}

private:
	int _value;
};

void log(const char *tag, const char *msg) {
	printf("%30s: %s\n", tag, msg);
}

void* producer_routine(void* arg) {
	const char *TAG = "Producer thread";
	Value *value = (Value*) arg;
	int tmp;
	pthread_mutex_t wait_for_consumer;
	pthread_mutex_t value_read_m;
	log(TAG, "created");

	// Wait for consumer to start
	log(TAG, "waiting for consumer");
	pthread_mutex_lock(&wait_for_consumer);
	while (!consumer_ready_cond)
		pthread_cond_wait(&consumer_ready, &wait_for_consumer);
	log(TAG, "consumer notified that he is ready");
	pthread_mutex_unlock(&wait_for_consumer);

	// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
	while (std::cin >> tmp) {
		log(TAG, "read a number");

		pthread_mutex_lock(&value_lock);
		value->update(tmp);
		log(TAG, "updated value");
		pthread_mutex_unlock(&value_lock);

		pthread_mutex_lock(&cond_lock);
		value_updated_cond = true;
		pthread_mutex_unlock(&cond_lock);
		log(TAG, "value_updated_cond = true");
		pthread_cond_broadcast(&value_updated);
		log(TAG, "notified consumer");
		if (tmp == -1)
			break;

		log(TAG, "waiting for accept");
		pthread_mutex_lock(&value_read_m);
		while (!value_read_cond)
			pthread_cond_wait(&value_read, &value_read_m);
		pthread_mutex_unlock(&value_read_m);

		pthread_mutex_lock(&cond_lock);
		value_read_cond = false;
		log(TAG, "consumer notified that he is ready");
		log(TAG, "value_read_cond = false");
		pthread_mutex_unlock(&cond_lock);
	}

	log(TAG, "exiting");
	return NULL;
}

void* consumer_routine(void* arg) {
	const char *TAG = "Consumer thread";
	Value *value = (Value*) arg;
	long sum;
	int tmp;
	pthread_mutex_t value_updated_m;
	log(TAG, "created");

	// notify about start
	pthread_mutex_lock(&cond_lock);
	consumer_ready_cond = true;
	log(TAG, "consumer_ready_cond = true");
	pthread_mutex_unlock(&cond_lock);
	pthread_cond_broadcast(&consumer_ready);

	// allocate value for result
	sum = 0;

	// for every update issued by producer, read the value and add to sum
	for (;;) {
		log(TAG, "waiting for value to update");
		pthread_mutex_lock(&value_updated_m);
		while (!value_updated_cond)
			pthread_cond_wait(&value_updated, &value_updated_m);
		pthread_mutex_unlock(&value_updated_m);

		pthread_mutex_lock(&cond_lock);
		value_updated_cond = false;
		log(TAG, "value_updated_cond = false");
		pthread_mutex_unlock(&cond_lock);

		log(TAG, "value updated");
		pthread_mutex_lock(&value_lock);
		tmp = value->get();
		log(TAG, "value read");
		pthread_mutex_unlock(&value_lock);

		if (tmp == -1)
			break;

		sum += tmp;
		log(TAG, "sum updated");

		pthread_mutex_lock(&cond_lock);
		value_read_cond = true;
		log(TAG, "value_read_cond = true");
		pthread_mutex_unlock(&cond_lock);
		pthread_cond_broadcast(&value_read);
		log(TAG, "notified producer");
	}

	// return pointer to result
	log(TAG, "exiting");
	return (void *) sum;
}

void* consumer_interruptor_routine(void* arg) {
	const char *TAG = "Interrupter thread";
	pthread_mutex_t wait_for_consumer;
	pthread_mutex_t value_updated_m;
	log(TAG, "created");

	// wait for consumer to start
	log(TAG, "waiting for consumer");
	pthread_mutex_lock(&wait_for_consumer);
	while (!consumer_ready_cond)
		pthread_cond_wait(&consumer_ready, &wait_for_consumer);
	log(TAG, "consumer notified that he is ready");
	pthread_mutex_unlock(&wait_for_consumer);

	for (;;) {
		log(TAG, "waiting for value to update");
		pthread_mutex_lock(&value_updated_m);
		while (!value_updated_cond)
			pthread_cond_wait(&value_updated, &value_updated_m);
		pthread_mutex_unlock(&value_updated_m);

		// interrupt consumer while producer is running
		log(TAG, "value updated");
		pthread_mutex_lock(&value_lock);
		log(TAG, "interrupting");
		pthread_mutex_unlock(&value_lock);
	}

	log(TAG, "exiting");
	return NULL;
}

int run_threads() {
	// start 3 threads and wait until they're done
	const char *TAG = "Main thread";
	Value value;
	void *ret;
	long sum;

	log(TAG, "starting threads");
	pthread_create(&prod_t, NULL, producer_routine, &value);
	pthread_create(&cons_t, NULL, consumer_routine, &value);
	pthread_create(&inter_t, NULL, consumer_interruptor_routine, NULL);

	log(TAG, "waiting for threads to exit");
	pthread_join(prod_t, NULL);
	pthread_join(cons_t, &ret);
	pthread_cancel(inter_t);

	// return sum of update values seen by consumer
	sum = (long) ret;
	log(TAG, "exiting");
	return sum;
}

int main() {
	std::cout << run_threads() << std::endl;
	return 0;
}
