#include <iostream>
#include <pthread.h>
#include <vector>
#include <memory>
#include <unistd.h>
#include <cassert>

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

std::shared_ptr<Value> value_ptr(new Value());

enum signal_status {
	INITIAL,
	VALUE_READY,
	VALUE_CONSUMED,
	NO_NEW_VALUE
};

signal_status status = signal_status::INITIAL;

void* consumer_routine(void* arg);

class consumer_t {
	pthread_t _consumer;
	bool _waiting_new_tasks;

public:
	explicit consumer_t()
	: _waiting_new_tasks(true) {
		pthread_create(&_consumer, nullptr, &consumer_routine, &value_ptr);
	}

	pthread_t consumer() const {
		return _consumer;
	}

	bool is_waiting_tasks() const {
		return _waiting_new_tasks;
	}

	void end() {
		_waiting_new_tasks = false;
	}
};

/*
 * Number of consumer threads (passed as command-line argument)
 */
uint32_t N;

/*
 * Maximal time that consumer may sleep (in microseconds)
 */
uint32_t max_sleeping_time;

/*
 * Mutex for thread-safe operations with value shared by producer and consumers
 */
pthread_mutex_t producer_consumer_mutex;

/*
 * Notification for consumers: producer put new value to shared variable
 */
pthread_cond_t new_value_prepared;

/*
 * Notification for producer: one of consumers updated sum
 */
pthread_cond_t value_consumed;

/*
 * Barrier for all N + 2 created threads;
 * producer, interruptor and consumers will do useful work
 * only after all of them will be created and started
 */
pthread_barrier_t consumers_created_barrier;

/*
 * producer, interruptor, and consumers, respectively
 */
pthread_t producer;
pthread_t interruptor;
std::vector<consumer_t> consumers;

/*
 * Flag for interruptor indicating that consumers will not do useful work anymore, so
 * it can stop trying to cancel them
 */
bool are_consumers_terminated() {
	for (const consumer_t &consumer : consumers) {
		if (consumer.is_waiting_tasks())
			return false;
	}
	return true;
}

/*
 * Signals consumers that producer will not produce new values, and they can stop working
 */
void stop_consumers() {
	pthread_mutex_lock(&producer_consumer_mutex);
	status = signal_status::NO_NEW_VALUE;
	for (consumer_t &consumer : consumers) {
		consumer.end();
	}
	// Joke for consumers: they notified about new value, woke up and suddenly
	pthread_cond_broadcast(&new_value_prepared);
	pthread_mutex_unlock(&producer_consumer_mutex);
}

void* producer_routine(void* arg) {
	// Wait for consumer to start
	pthread_barrier_wait(&consumers_created_barrier);

	std::shared_ptr<Value> value = *reinterpret_cast<std::shared_ptr<Value>*>(arg);
	// Read data, loop through each value and update the value, notify consumer, wait for consumer to process
	int next_value;
	while (std::cin >> next_value) {
		pthread_mutex_lock(&producer_consumer_mutex);
		value->update(next_value);
		status = signal_status::VALUE_READY;

		pthread_cond_signal(&new_value_prepared);

		while (status != signal_status::VALUE_CONSUMED) {
			pthread_cond_wait(&value_consumed, &producer_consumer_mutex);
		}
		pthread_mutex_unlock(&producer_consumer_mutex);
	}
	stop_consumers();
	return nullptr;
}

void sleep_for_random_time(uint32_t max_sleeping_time) {
	assert(max_sleeping_time > 0);
	uint32_t sleeping_time = ((uint32_t) random() % max_sleeping_time) * 1000;
	usleep(sleeping_time);
}

void* consumer_routine(void* arg) {

	// notify about start
	pthread_barrier_wait(&consumers_created_barrier);

	// Prevent consumer from cancelling diversions of interruptor_routine
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

	std::shared_ptr<Value> value = *reinterpret_cast<std::shared_ptr<Value>*>(arg);

	int sum = 0;

	// for every update issued by producer, read the value and add to sum
	// return pointer to result (aggregated result for all consumers)
	while (true) {
		pthread_mutex_lock(&producer_consumer_mutex);

		if (status == signal_status::NO_NEW_VALUE) {
			pthread_mutex_unlock(&producer_consumer_mutex);
			break;
		}

		while (status == signal_status::INITIAL || status == signal_status::VALUE_CONSUMED) {
			pthread_cond_wait(&new_value_prepared, &producer_consumer_mutex);
		}

		if (status == signal_status::VALUE_READY) {
			sum += value->get();
			status = signal_status::VALUE_CONSUMED;
		}
		pthread_cond_signal(&value_consumed);
		pthread_mutex_unlock(&producer_consumer_mutex);
		if (max_sleeping_time > 0) {
			sleep_for_random_time(max_sleeping_time);
		}
	}

	// Allow consumer to be cancelled
	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, nullptr);
	return new int(sum);
}

void* consumer_interruptor_routine(void* arg) {
	// wait for consumer to start
	pthread_barrier_wait(&consumers_created_barrier);

	// interrupt consumer while producer is running
	while (!are_consumers_terminated()) {
		pthread_cancel(consumers[random() % N].consumer());
	}
	return nullptr;
}

int run_threads() {
	// start N threads and wait until they're done
	// return aggregated sum of values
	pthread_mutex_init(&producer_consumer_mutex, nullptr);
	pthread_cond_init(&new_value_prepared, nullptr);
	pthread_cond_init(&value_consumed, nullptr);

	// 1 producer + 1 interruptor + N consumers
	pthread_barrier_init(&consumers_created_barrier, nullptr, N + 2);
	pthread_create(&producer, nullptr, &producer_routine, &value_ptr);
	for (int i = 0; i < N; ++i) {
		consumers.emplace_back(consumer_t());
	}
	pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, &value_ptr);

	int* sum;
	int result = 0;
	pthread_join(producer, nullptr);
	for (int i = 0; i < N; ++i) {
		pthread_join(consumers[i].consumer(), (void**)(&sum));
		std::cout << "Consumer " << i << " delivered sum " << *sum << std::endl;
		result += *sum;
	}
	pthread_join(interruptor, nullptr);
	delete sum;
	return result;
}

int main(int argc, char** argv) {
	if (argc != 3) {
		std::cerr << "You must pass only 2 parameters: number of consumers and max slepping time (in ms)." << std::endl;
	}
	N = static_cast<uint32_t>(std::atoi(argv[1]));
	max_sleeping_time = static_cast<uint32_t>(std::atoi(argv[2]));
	int result = run_threads();
	std::cout << "\n===\nTotal sum: " << result << std::endl;
	return 0;
}