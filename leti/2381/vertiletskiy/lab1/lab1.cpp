#include <pthread.h>
#include <iostream>
#include <stdlib.h>
#include <vector>

using namespace std;

class Value {
public:
	Value() : _value(0) { }

    void Update(int value) 
	{
        _value = value;
	}	

    int Get() const 
	{
        return _value;
	}
		
private:
    int _value;
};

pthread_mutex_t mutex;
pthread_cond_t consumerCond;
pthread_cond_t valueUpdatedCond;

bool consumerStarted = false;
bool valueUpdated = false;
bool isFinished = false;

Value *value;
vector<int>* inputValues;

void* ProducerRoutine(void* arg) 
{
	cout<<"Producer was started..."<<endl;

	pthread_mutex_lock(&mutex);
	if(!consumerStarted)
	{
		pthread_cond_wait(&consumerCond, &mutex);
	}
	pthread_mutex_unlock(&mutex);

	for(int i = 0; i < inputValues->size(); i++)
	{
		pthread_mutex_lock(&mutex);
			if(valueUpdated) pthread_cond_wait(&consumerCond, &mutex);

			cout<<"Value updated from "<<value->Get();
			value->Update(inputValues->at(i));
			cout<<" to "<<value->Get()<<endl;

			valueUpdated = true;
			pthread_cond_signal(&valueUpdatedCond);
		pthread_mutex_unlock(&mutex);
	}

	isFinished = true;
}

void* ConsumerRoutine(void* arg) 
{
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);

	cout<<"Consumer was started..."<<endl;
	pthread_mutex_lock(&mutex);
	consumerStarted = 1;
	pthread_cond_broadcast(&consumerCond);
	pthread_mutex_unlock(&mutex);

	int* sum = (int*) arg;
	do
	{
		pthread_mutex_lock(&mutex);
		
		if(!valueUpdated) pthread_cond_wait(&valueUpdatedCond, &mutex);

		(*sum) += value->Get();
		cout<<"Added "<<value->Get()<<endl;
		valueUpdated = false;
		pthread_cond_signal(&consumerCond);
		pthread_mutex_unlock(&mutex);

	} while(!isFinished);

}

void* ConsumerInterruptorRoutine(void* arg) 
{
	cout<<"Interruptor was started..."<<endl;

	pthread_mutex_lock(&mutex);

	if(!consumerStarted) pthread_cond_wait(&consumerCond, &mutex);

	pthread_mutex_unlock(&mutex);

	pthread_t* consumerThread = (pthread_t*) arg;

	while(!isFinished) pthread_cancel(*consumerThread);
}


int RunThreads() 
{
	pthread_t producerThread, consumerThread, consumerInterruptorThread;
	int result = 0;
	
	pthread_create(&producerThread, NULL, ProducerRoutine, NULL);
	pthread_create(&consumerThread, NULL, ConsumerRoutine, &result);
	pthread_create(&consumerInterruptorThread, NULL, ConsumerInterruptorRoutine, &consumerThread);

	pthread_join(producerThread, NULL); 
	pthread_join(consumerThread, NULL); 
	pthread_join(consumerInterruptorThread, NULL); 

  	return result;
}

int main(int argc, char *argv[])
{
	cout << "Program started" << endl;

	if(argc > 1) 
	{
		pthread_mutex_init(&mutex, NULL);
		pthread_cond_init(&consumerCond, NULL);
		pthread_cond_init(&valueUpdatedCond, NULL);
		value= new Value();

		inputValues = new vector<int>(argc-1);
		
		for(int i = 0; i < inputValues->size() ; i++)
		{
			inputValues->at(i) = atoi(argv[i+1]);
		}	
		cout << "Sum is "<< RunThreads() << endl;
	} 
	else 
	{
		cout << "There's no input arguments!" << endl;
	} 	
    return 0;
}