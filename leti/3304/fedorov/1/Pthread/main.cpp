#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
using namespace std;

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

bool IsConsumerStart = false;
bool IsConsumerReadSharedValue = false;
bool IsSharedDataChanged = false;

void* producer_routine(void* sharedValuePtr) {
	while (!IsConsumerStart);
	string input;
	getline(cin, input);
	stringstream ss;
	ss << input;
	while (true) {
		int number;
		if (!(ss >> number))
			break;
		*((int *)sharedValuePtr) = number;//заменим int на Value
		IsSharedDataChanged = true;
		while (!IsConsumerReadSharedValue);
	}
	return NULL;
}

void* consumer_routine(void* sharedValuePtr) {
	IsConsumerStart = true;
	while (true)
	{
		while (!IsSharedDataChanged);
		IsSharedDataChanged = false;
		//добавим *sharedValuePtr к сумме
	}
}

void* consumer_interruptor_routine(void* arg) {
	// wait for consumer to start

	// interrupt consumer while producer is running
	return NULL;
}

int run_threads(int consumersCount) {
	if (consumersCount <= 0)
		throw new exception();
	pthread_t producerID;
	pthread_create(&producerID, NULL, producer_routine, NULL);//передадим sharedValuePtr
	pthread_t interruptorID;
	pthread_create(&interruptorID, NULL, consumer_interruptor_routine, NULL);
	vector<pthread_t> consumersID;
	for (int i = 0; i < consumersCount; i++)
	{
		pthread_t id;
		consumersID.push_back(id);
		pthread_create(&id, NULL, consumer_routine, NULL);//передадим sharedValuePtr
	}
	pthread_join(producerID, NULL);
	pthread_join(interruptorID, NULL);
	void *result;
	pthread_join(consumersID.at(0), &result);
	for (int i = 1; i < consumersCount; i++)
		pthread_join(consumersID.at(i), NULL);
	return (*(int *)result);
}

int main(int argc, char *argv[]) {
	cout << run_threads(atoi(argv[1])) << endl;
	system("pause");
	return 0;
}