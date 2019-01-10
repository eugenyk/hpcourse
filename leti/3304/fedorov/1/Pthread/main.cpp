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
bool IsSharedDataChanged = false;
bool IsInputEnded = false;
pthread_mutex_t Mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t DataChangedCondition = PTHREAD_COND_INITIALIZER;
pthread_cond_t ConsumerStartCondition = PTHREAD_COND_INITIALIZER;
int ConsumersCount;

void* producer_routine(void* sharedValuePtr) {
	pthread_mutex_lock(&Mutex);
	while (!IsConsumerStart);
	pthread_mutex_unlock(&Mutex);
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
		pthread_mutex_lock(&Mutex);
		((Value *)sharedValuePtr)->update(number);
		IsSharedDataChanged = true;
        	pthread_cond_broadcast(&DataChangedCondition);
           	pthread_cond_wait(&ConsumerStartCondition, &Mutex);
        	IsSharedDataChanged = false;
		pthread_mutex_unlock(&Mutex);
	}
	pthread_mutex_lock(&Mutex);
    	IsSharedDataChanged = true;
    	pthread_cond_broadcast(&DataChangedCondition);
	pthread_mutex_unlock(&Mutex);
	return NULL;
}

void* consumer_routine(void* sharedValuePtr) {
	IsConsumerStart = true;
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	static int count = 0;
    	Value *sum = new Value();
	while (true)
	{
		pthread_mutex_lock(&Mutex);
        	count++;
        	if (count >= ConsumersCount)
        	{
            		pthread_cond_broadcast(&ConsumerStartCondition);
            		count = 0;
        	}
        	do
		{
            		pthread_cond_wait(&DataChangedCondition, &Mutex);
		}while (!IsSharedDataChanged);
		if (IsInputEnded)
		{
			pthread_mutex_unlock(&Mutex);
			break;
		}
		sum->update(sum->get() + ((Value *)sharedValuePtr)->get());
		pthread_mutex_unlock(&Mutex);
		usleep(rand() % (MaxSleepTime * 1000 + 1));
//		usleep(1000);
	}
	return (void *)sum;
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

int run_threads() {
	Value sharedValue;
	pthread_t producerID;
	pthread_create(&producerID, NULL, producer_routine, &sharedValue);
	srand(time(NULL));
	vector<pthread_t> consumersID;
	consumersID.resize(ConsumersCount);
	for (int i = 0; i < ConsumersCount; i++)
		pthread_create(&consumersID[i], NULL, consumer_routine, &sharedValue);
	pthread_t interruptorID;
	pthread_create(&interruptorID, NULL, consumer_interruptor_routine, &consumersID);
	pthread_join(producerID, NULL);
	pthread_join(interruptorID, NULL);
	void *result;
	pthread_join(consumersID.at(0), &result);
	for (int i = 1; i < ConsumersCount; i++)
		pthread_join(consumersID.at(i), NULL);
	return *((int *)result);
}

int main(int argc, char *argv[]) {
	ConsumersCount = atoi(argv[1]);
	MaxSleepTime = atoi(argv[2]);
	cout << run_threads() << endl;
//	system("pause");
	return 0;
}
