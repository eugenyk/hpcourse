#include <iostream>
#include <memory>
#include <pthread.h>

class pthread_lock_guard {
public:
	pthread_lock_guard(pthread_mutex_t* lock): lock(lock) {
		pthread_mutex_lock(lock);
	}

	~pthread_lock_guard() {
		pthread_mutex_unlock(lock);
	}

private:
	pthread_mutex_t* lock;
};

using value_t = unsigned long long;

class Value {
public:
	Value() : has_value(false), started(false), over(false), value(0) {
		pthread_mutex_init(&mutex, nullptr);
		pthread_cond_init(&cond_empty, nullptr);
		pthread_cond_init(&cond_full, nullptr);
	}

	~Value() {
		pthread_cond_destroy(&cond_full);
		pthread_cond_destroy(&cond_empty);
		pthread_mutex_destroy(&mutex);
	}

	void update(value_t new_value) {
		pthread_lock_guard guard(&mutex);

		while (!started || has_value)
			pthread_cond_wait(&cond_empty, &mutex);
		has_value = true;
		value = new_value;
		pthread_cond_signal(&cond_full);
	}

	bool get(value_t& res) {
		pthread_lock_guard guard(&mutex);

		while (!has_value && !over)
			pthread_cond_wait(&cond_full, &mutex);
		if (over)
			return false;
		has_value = false;
		res = value;
		pthread_cond_signal(&cond_empty);
		return true;
	}

	void start()  {
		pthread_lock_guard guard(&mutex);
		started = true;
		pthread_cond_broadcast(&cond_empty);
	}

	void stop() {
		pthread_lock_guard guard(&mutex);
		while (has_value)
			pthread_cond_wait(&cond_empty, &mutex);
		over = true;
		pthread_cond_broadcast(&cond_full);
	}

	bool is_working() const {
		pthread_lock_guard guard(&mutex);
		return started && !over;
	}

private:
	bool has_value, started, over;
	value_t value;
	mutable pthread_mutex_t mutex;
	mutable pthread_cond_t cond_empty;
	mutable pthread_cond_t cond_full;
};

void* producer_routine(void* arg) {
	Value& value = *static_cast<Value*>(arg);
	while (true) {
		value_t v;
		std::cin >> v;
		if (std::cin.eof())
			break;
		value.update(v);
	}
	value.stop();
	return nullptr;
}

void* consumer_routine(void* arg) {
	Value& value = *static_cast<Value*>(arg);
	value_t& result = *new value_t(0);
	int oldstate;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
	value.start();
	while (true) {
		value_t v;
		if (!value.get(v))
			break;
		result += v;
	}
	pthread_setcancelstate(oldstate, nullptr);
	return &result;
}

struct interruptor_data {
	Value& value;
	pthread_t thread;
};

void* consumer_interruptor_routine(void* arg) {
	interruptor_data& data = *static_cast<interruptor_data*>(arg);
	int i(0);
	while (data.value.is_working()) {
		pthread_cancel(data.thread);
		++i;
	}
	std::cerr << "Shot consumer " << i << " times!" << std::endl;
	return nullptr;
}

value_t run_threads() {
	Value value;
	pthread_t producer, consumer, interruptor;

	pthread_create(&producer, nullptr, &producer_routine, &value);
	pthread_create(&consumer, nullptr, &consumer_routine, &value);
	interruptor_data int_d{value, consumer};
	pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, &int_d);

	void* result_p_;
	pthread_join(producer, nullptr);
	pthread_join(consumer, &result_p_);
	pthread_join(interruptor, nullptr);
	value_t* result_p(static_cast<value_t*>(result_p_));

	value_t result(*result_p);
	delete result_p;
	return result;
}

int main() {
	std::cout << run_threads() << std::endl;
	return 0;
}

