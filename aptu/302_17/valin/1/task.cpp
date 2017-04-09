#include <pthread.h>
#include <iostream>
#include <exception>
 
pthread_barrier_t barrier;
pthread_mutex_t mutex;

pthread_cond_t updated_cond;
bool updated;

pthread_cond_t got_cond;
bool got;

bool finished;

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
	Value *val = static_cast<Value*>(arg);
	int n = 1;

	pthread_barrier_wait(&barrier);

	while (std::cin >> n) {
		pthread_mutex_lock(&mutex);

		val->update(n);
		updated = true;
		pthread_cond_signal(&updated_cond);

		while (!got) {
			pthread_cond_wait(&got_cond, &mutex);
		}
		got = false;

		pthread_mutex_unlock(&mutex);
	}

	pthread_mutex_lock(&mutex);
	finished = true;
	pthread_cond_signal(&updated_cond);
	pthread_mutex_unlock(&mutex);

	return NULL;
}

void* consumer_routine(void* arg) {
	Value *val = static_cast<Value*>(arg);
	Value *sum = new Value();
	int oldstate;

	pthread_barrier_wait(&barrier);

	while (true) {
		pthread_mutex_lock(&mutex);

		while (!updated && !finished) {
			pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, &oldstate);
			pthread_cond_wait(&updated_cond, &mutex);
			pthread_setcancelstate(oldstate, &oldstate);
		}
		updated = false;
		if (finished) {
			pthread_mutex_unlock(&mutex);
			break;
		}

		sum->update(sum->get() + val->get());
		got = true;
		pthread_cond_signal(&got_cond);

		pthread_mutex_unlock(&mutex);
	}


	return static_cast<void*>(sum);
}

void* consumer_interruptor_routine(void* arg) {
	pthread_t *consumer = static_cast<pthread_t*>(arg);

	pthread_barrier_wait(&barrier);
	
	while (true) {
		pthread_mutex_lock(&mutex);
		if (finished) {
			pthread_mutex_unlock(&mutex);
			break;
		}
		pthread_mutex_unlock(&mutex);
		pthread_cancel(*consumer);
	}

	return NULL;
}

int run_threads() {
	Value value;
	Value *res;
	pthread_t threads[3];
	
	if (pthread_barrier_init(&barrier, NULL, 3))
		throw std::runtime_error("can't init barrier");

	if (pthread_cond_init(&updated_cond, NULL))
		throw std::runtime_error("can't init updated_cond");

	if (pthread_cond_init(&got_cond, NULL))
		throw std::runtime_error("can't init got_cond");

	if (pthread_create(&threads[0], NULL, producer_routine, static_cast<void*>(&value)))
		throw std::runtime_error("can't start producer_routine");

	if (pthread_create(&threads[1], NULL, consumer_routine, static_cast<void*>(&value)))
		throw std::runtime_error("can't start consumer_routine");

	if (pthread_create(&threads[2], NULL, consumer_interruptor_routine, static_cast<void*>(&threads[1])))
		throw std::runtime_error("can't start consumer_interruptor_routine");

	if (pthread_join(threads[0], NULL))
		throw std::runtime_error("can't join producer_routine");
		
	if (pthread_join(threads[1], reinterpret_cast<void**>(&res)))
		throw std::runtime_error("can't join consumer_routine");

	if (pthread_join(threads[2], NULL))
		throw std::runtime_error("can't join consumer_interruptor_routine");

	pthread_barrier_destroy(&barrier);
	pthread_mutex_destroy(&mutex);
	pthread_cond_destroy(&updated_cond);
	pthread_cond_destroy(&got_cond);

	return res->get();
}

int main() {
	try {
		std::cout << run_threads() << std::endl;
	} catch (std::exception& e) {
		std::cerr << "exception:" << e.what() << '\n';
	}

	return 0;
}
