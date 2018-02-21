#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
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


FILE *fIn, *fOut;

Value buffer; 

pthread_mutex_t mutex, mutexStartConsumer, mutexProducerWaitedConsumer;

pthread_t idProducer, idConsumer, idConsumerInterrupter;

pthread_cond_t condStatusUpdate, condConsumerStart, condProducerWaitedConsumer;

bool isConsumerTime = false;
bool consumerStart = false;
bool lastUpdate = false;
bool producerWaitedConsumer = false;

void wait_signal(pthread_mutex_t *mutex, 
	bool *isReally, pthread_cond_t *cond)
{
	pthread_mutex_lock(mutex);
    while(!*isReally)
    {
        pthread_cond_wait(cond, mutex);
    }
    pthread_mutex_unlock(mutex);

}

void send_signal(bool *isReally, pthread_cond_t *cond)
{
	*isReally = true;
    pthread_cond_signal(cond);
}

void* producer_routine(void* arg) {

	wait_signal(&mutexStartConsumer, &consumerStart, &condConsumerStart);
	pthread_mutex_lock(&mutexProducerWaitedConsumer);
	send_signal(&producerWaitedConsumer, &condProducerWaitedConsumer);
	pthread_mutex_unlock(&mutexProducerWaitedConsumer);
	while(!lastUpdate)
	{	
		pthread_mutex_lock(&mutex);
		while(isConsumerTime)
		{
			pthread_cond_wait(&condStatusUpdate, &mutex);	
		}
		int c = 0;
		if ( !feof(fIn) )
		{
			fscanf (fIn, "%d", &c);
			buffer.update(c);	
		}
		else
		{
			lastUpdate = true;	
		}
		isConsumerTime = true;
		pthread_cond_signal(&condStatusUpdate);
		pthread_mutex_unlock(&mutex);
	}
  // Wait for consumer to start
}
 
void* consumer_routine(void* arg) {
	pthread_mutex_lock(&mutexStartConsumer);
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	send_signal(&consumerStart, &condConsumerStart);
	pthread_mutex_unlock(&mutexStartConsumer);

	int* sum = (int*) malloc(sizeof(int));

	while(!lastUpdate)
	{
		pthread_mutex_lock(&mutex);
		while(!isConsumerTime)
		{
			pthread_cond_wait(&condStatusUpdate, &mutex);	
		}
		if(!lastUpdate)
		{	
			*sum+=buffer.get();//TODO	
		}
		isConsumerTime = false;
		pthread_cond_signal(&condStatusUpdate);
		pthread_mutex_unlock(&mutex);
	}	
	
	return sum;
  // notify about start
  // allocate value for result
  // for every update issued by producer, read the value and add to sum
  // return pointer to result
}
 
void* consumer_interruptor_routine(void* arg) {
	wait_signal(&mutexProducerWaitedConsumer, &producerWaitedConsumer,
		&condProducerWaitedConsumer);
	while(!lastUpdate)	
	{
		pthread_cancel(idConsumer);
	}
  // wait for consumer to start
 
  // interrupt consumer while producer is running                                          
}
 
int run_threads() {
  // start 3 threads and wait until they're done
  // return sum of update values seen by consumer
 
	void* sum;

	pthread_create( &idProducer, NULL,
                               producer_routine, NULL);
    pthread_create( &idConsumer, NULL,
                               consumer_routine, NULL);
    pthread_create( &idConsumerInterrupter, NULL,
	             	                   consumer_interruptor_routine, NULL);

	pthread_join(idProducer, NULL);
    pthread_join(idConsumer, &sum);
    pthread_join(idConsumerInterrupter, NULL);

  	return *(int*) sum;
}
 
int main(int argc, char *argv[]) {
	if( argc != 3 )
	{
		printf( "Invalud number arguments\n" );
		return 0;
	}

	const char * const FILE_IN = argv[1];
	const char * const FILE_OUT = argv[2];

	fIn = fopen(FILE_IN,"r");
	fOut = fopen(FILE_OUT,"w");

	pthread_cond_init(&condStatusUpdate, NULL);
	pthread_cond_init(&condConsumerStart, NULL);
	pthread_cond_init(&condProducerWaitedConsumer, NULL);


	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutexStartConsumer, NULL);
	pthread_mutex_init(&mutexProducerWaitedConsumer, NULL);

	fprintf(fOut, "%d", run_threads());

	fclose(fIn);
	fclose(fOut);
	return 0;
}
