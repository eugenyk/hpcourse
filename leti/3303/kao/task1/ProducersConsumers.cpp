#include <iostream>
#include <pthread.h>
#include <sstream>
#include "unistd.h"
#include <vector>

using namespace std;
class Information {
public:
	Information() {
		this->info = 0;
	}

	void update(int info) {
		this->info = info;
	}
	int get() const {
		return this->info;
	}

private:
	int info;

};


namespace {
	pthread_barrier_t barrier;
	pthread_mutex_t mutex;
	pthread_cond_t cond;

	volatile bool flag = false;
	volatile bool isFinished = false;
	unsigned maxSleepTime = 0;

}

void* producer(void* arg)
{
	Information& info = *static_cast<Information*>(arg);
	pthread_barrier_wait(&barrier);
	string inputInfo;
	getline(cin, inputInfo);
	istringstream iss(inputInfo);

	do 
	{
		int newValue = 0;
		iss >> newValue;
		pthread_mutex_lock(&mutex);
		while (flag)
		{
			pthread_cond_wait(&cond, &mutex);
		}
		info.update(newValue);
		isFinished = iss.eof();
		flag = true;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal(&cond);

	} while (!isFinished);

	pthread_cond_broadcast(&cond);
	pthread_exit(nullptr);

}

void* consumer(void* arg)
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
	const auto& info = *static_cast<Information*>(arg);
	static int* sum = new int{ 0 };
	auto getSleepTime = []() -> useconds_t {
		return maxSleepTime ? static_cast<useconds_t>(rand()) % maxSleepTime : 0;
	};
	pthread_barrier_wait(&barrier);
	do {
		pthread_mutex_lock(&mutex);
		while (!flag && !isFinished)
			pthread_cond_wait(&cond, &mutex);

		if (!flag) {
			pthread_mutex_unlock(&mutex);
			pthread_exit(sum);
		}

		(*sum) += info.get();

		flag = false;
		pthread_mutex_unlock(&mutex);
		pthread_cond_broadcast(&cond);
		usleep(getSleepTime());
	} while (true);

}


void* consumer_interruptor(void* arg)
{
	const auto& consumers = *static_cast<std::vector<pthread_t>*>(arg);
	pthread_barrier_wait(&barrier);

	while (!isFinished) {
		unsigned long index = static_cast<unsigned long>(random()) % consumers.size();
		pthread_cancel(consumers[index]);
		usleep(1);
	}

	pthread_exit(nullptr);
}


int run_threads(int consummersCount)
{
	// todo check retval and exit if not null
	pthread_barrier_init(&barrier, nullptr, static_cast<unsigned>(consummersCount + 2));
	pthread_mutex_init(&mutex, nullptr);
	pthread_cond_init(&cond, nullptr);

	Information info;

	pthread_t producer;
	pthread_t interaptor;
	std::vector<pthread_t> consumers(static_cast<unsigned long>(consummersCount));

	pthread_create(&producer, nullptr, &producer, &info);
	pthread_create(&interaptor, nullptr, &consumer_interruptor, &consumers);
	for (pthread_t& consumer : consumers)
		pthread_create(&consumer, nullptr, &consumer, &info);

	pthread_join(producer, nullptr);
	pthread_join(interaptor, nullptr);
	void* consumerRetval = nullptr;
	for (const pthread_t& consumer : consumers)
		pthread_join(consumer, &consumerRetval);

	int sum = 0;
	if (nullptr != consumerRetval) {
		int* consumerRetvalInt = static_cast<int*>(consumerRetval);
		sum = *consumerRetvalInt;
		delete consumerRetvalInt;
	}

	return sum;
}

int main(int argc, char** argv)
{
	if (argc != 3) {
		std::cerr << "Must 2 cmd-line arguments." << std::endl;
		return -1;
	}

	int consumersCount = atoi(argv[1]);
	maxSleepTime = static_cast<unsigned>(atoi(argv[2]));

	if (consumersCount <= 0) {
		std::cerr << "Illigal 1-st argument. Must consumers count greather than zerro." << std::endl;
		return -2;
	}

	std::cout << run_threads(consumersCount) << std::endl;
	return 0;
}