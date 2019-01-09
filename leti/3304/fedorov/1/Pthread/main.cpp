#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#include <iostream>
#include <cstdlib>
#include <vector>
#include <string>
#include <sstream>
//#include <windows.h>
#include <unistd.h>
#include <ctime>
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

int MaxSleepTime;
bool IsConsumerStart = false;
bool IsConsumerReadSharedValue = false;
bool IsSharedDataChanged = false;
bool IsInputEnded = false;
int Sum = 0;

void* producer_routine(void* sharedValuePtr) {
	while (!IsConsumerStart);
	string input;
	getline(cin, input);
	stringstream ss;
	ss << input;
	while (true) {
		int number;
		if (!(ss >> number))
		{
			IsInputEnded = true;
			break;
		}
		((Value *)sharedValuePtr)->update(number);
		IsSharedDataChanged = true;
		while (!IsConsumerReadSharedValue);
		IsConsumerReadSharedValue = false;
	}
	return NULL;
}

void* consumer_routine(void* sharedValuePtr) {
	IsConsumerStart = true;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	while (true)
	{
		while (!IsSharedDataChanged)
		{
			if (IsInputEnded)
				break;
		}
		if (IsInputEnded)
			break;
		IsSharedDataChanged = false;
		Sum += ((Value *)sharedValuePtr)->get();
		IsConsumerReadSharedValue = true;
		usleep(rand() % (MaxSleepTime * 1000 + 1));
//		usleep(1000);
	}
	return (void *)&Sum;
}

void* consumer_interruptor_routine(void* consumersIDPtr) {
	while (!IsConsumerStart);
	while (true)
	{
		vector<pthread_t> consumersID = *((vector<pthread_t> *)consumersIDPtr);
		int i = rand() % (consumersID.size());
		pthread_cancel(consumersID.at(i));
		if (IsInputEnded)
			break;
	}
	return NULL;
}

int run_threads(int consumersCount) {
	if (consumersCount <= 0)
		throw new exception();
	Value sharedValue;
	pthread_t producerID;
	pthread_create(&producerID, NULL, producer_routine, &sharedValue);
	srand(time(NULL));
	vector<pthread_t> consumersID;
	consumersID.resize(consumersCount);
	for (int i = 0; i < consumersCount; i++)
		pthread_create(&consumersID[i], NULL, consumer_routine, &sharedValue);
	pthread_t interruptorID;
	pthread_create(&interruptorID, NULL, consumer_interruptor_routine, &consumersID);
	pthread_join(producerID, NULL);
	pthread_join(interruptorID, NULL);
	void *result;
	pthread_join(consumersID.at(0), &result);
	for (int i = 1; i < consumersCount; i++)
		pthread_join(consumersID.at(i), NULL);
	return *((int *)result);
}

int main(int argc, char *argv[]) {
	MaxSleepTime = atoi(argv[2]);
	cout << run_threads(atoi(argv[1])) << endl;
//	system("pause");
	return 0;
}
