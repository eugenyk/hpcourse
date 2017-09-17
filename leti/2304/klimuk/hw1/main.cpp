#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>

class Value {
	public:
		Value() : _value(0) {}
		
		void update(int value) {
			_value = value;
		}
		
		int get() const {
			return _value;
		}
		
	private:
		int _value;			
};


pthread_mutex_t m_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t can_consume = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_produce = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_is_started = PTHREAD_COND_INITIALIZER;

//For disable spurious wakeups
bool b_can_consume = false;
bool b_can_produce = false;
bool b_consumer_is_started = false;

pthread_t t_producer;
pthread_t t_consumer;
pthread_t t_interruptor;

pthread_barrier_t prod_inter_barrier;
bool producer_finish = false;


void clean(void* arg)
{
	pthread_mutex_unlock(&m_mutex);
}

void consumer_sum_clean(void *arg)
{
	pthread_mutex_unlock(&m_mutex);
	delete arg;
}

void *producer_routine(void *arg) {
	//Wait for consumer to start
	pthread_mutex_lock(&m_mutex);
	pthread_cleanup_push(clean, NULL)

	//std::cout << "Producer: waiting for consumer to start..." << std::endl;
	while (!b_consumer_is_started)
		pthread_cond_wait(&consumer_is_started, &m_mutex);

	pthread_cleanup_pop(0)
	pthread_mutex_unlock(&m_mutex);

	//Init start values
	Value *value = reinterpret_cast<Value *>(arg);
	
	std::vector<int> in_values;
	int input;

	//Read input	
	std::string line;
	getline(std::cin, line);
	std::istringstream is(line);

	//transform string to int array
	while (is >> input) {
		in_values.emplace_back(input);
	}

	//loop through each value and update the value, notify consumer, wait for consumer to process
	for (auto next_int : in_values) {
		pthread_mutex_lock(&m_mutex);
		pthread_cleanup_push(clean, NULL)
		pthread_barrier_wait(&prod_inter_barrier);

		value->update(next_int);

		//std::cout << "Producer: send update signal..." << std::endl;
		b_can_consume = true;
		b_can_produce = false;
		pthread_cond_signal(&can_consume);
		
		//std::cout << "Producer: waiting for consumer to process..." << std::endl;
		while(!b_can_produce)
			pthread_cond_wait(&can_produce, &m_mutex);

		pthread_cleanup_pop(0)
		pthread_mutex_unlock(&m_mutex);
	}
	
	//std::cout << "Producer: finish..." << std::endl;
	producer_finish = true;
	pthread_barrier_wait(&prod_inter_barrier);
	b_can_consume = true;
	pthread_cond_signal(&can_consume);

	return nullptr;
}

void *consumer_routine(void *arg) {
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	// notify about start
	pthread_mutex_lock(&m_mutex);
	pthread_cleanup_push(clean, NULL);

	//std::cout << "Consumer: send signal about start..." << std::endl;
	b_consumer_is_started = true;
	pthread_cond_broadcast(&consumer_is_started);

	pthread_cleanup_pop(0);
	pthread_mutex_unlock(&m_mutex);

	// allocate value for result
	int *sum = new int;
	*sum = 0;

	// for every update issued by producer, read the value and add to sum
	Value *value = reinterpret_cast<Value *>(arg);
	while (!producer_finish)
	{
		pthread_mutex_lock(&m_mutex);
		pthread_cleanup_push(consumer_sum_clean, (void*) sum);
		
		//std::cout << "Consumer: waiting for producer to update..." << std::endl;
		while (!b_can_consume)
			pthread_cond_wait(&can_consume, &m_mutex);

		*sum += value->get();

		//std::cout << "Consumer: send signal about end process..." << std::endl;
		b_can_consume = false;
		b_can_produce = true;
		pthread_cond_signal(&can_produce);

		if (producer_finish)
			*sum -= value->get();

		pthread_cleanup_pop(0);
		pthread_mutex_unlock(&m_mutex);	
	}

	//std::cout << "Consumer: finish..." << std::endl;
	// return pointer to result
	return reinterpret_cast<void *>(sum);
}

void *consumer_interruptor_routine(void *arg) {
	// wait for consumer to start	
	pthread_mutex_lock(&m_mutex);
	pthread_cleanup_push(clean, NULL)

	//std::cout << "Interruptor: waiting for consumer to start..." << std::endl;
	while(!b_consumer_is_started)
		pthread_cond_wait(&consumer_is_started, &m_mutex);
	
	pthread_cleanup_pop(0);
	pthread_mutex_unlock(&m_mutex);
	// interrupt consumer while producer is running
	while (!producer_finish)
	{	
		pthread_barrier_wait(&prod_inter_barrier);
		if (!producer_finish) {
			//std::cout << "Interruptor: try to interrupt while produce..." << std::endl;
			pthread_cancel(t_consumer);
		}
	}

	return nullptr;
}

int run_threads() {
	Value *value = new Value();
	int *res;

	pthread_barrier_init(&prod_inter_barrier, nullptr, 2);
	// start 3 threads and wait until they're done
	pthread_create(&t_producer, nullptr, &producer_routine, value);
	pthread_create(&t_interruptor, nullptr, &consumer_interruptor_routine, nullptr);
	pthread_create(&t_consumer, nullptr, &consumer_routine, value);

	// return sum of update values seen by consumer
	pthread_join(t_producer, nullptr);
	pthread_join(t_interruptor, nullptr);
	pthread_join(t_consumer, (void **)&res);

	int result = *res;

	delete value;
	delete res;
	
	return result;
}

int main() {
	std::cout << run_threads() << std::endl;
	//system("pause");
	return 0;
}