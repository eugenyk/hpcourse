//============================================================================
// Name        : Lab_1.cpp
// Author      : Kristina Gridneva
// Version     :
// Copyright   : Your copyright notice
// Description : Lab_1 in C++, Ansi-style
//============================================================================

#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <pthread.h>
#include <unistd.h>
using namespace std;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER; //mutex for cond1 & cond2
pthread_mutex_t mutex2 = PTHREAD_MUTEX_INITIALIZER; //mutex for ready
pthread_mutex_t mutex3 = PTHREAD_MUTEX_INITIALIZER; //mutex for finished producer
pthread_cond_t cond1 =  PTHREAD_COND_INITIALIZER;  //condition for consumer
pthread_cond_t cond2 =  PTHREAD_COND_INITIALIZER;  //condition for producer
pthread_cond_t ready =  PTHREAD_COND_INITIALIZER;  //condition for consumer to producer & interruptor start
bool update_value = false;
bool ready_consumer = false;
bool finished_producer = false;
int sleep_time;
int threads_num;
int sum = 0;
int iter_num = 0;
int numbers = -1;

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

std::vector<std::string> split(const std::string& s, char delimiter)
{
  std::vector<std::string> tokens;
  std::string token;
  std::istringstream tokenStream(s);
  while (std::getline(tokenStream, token, delimiter))
  {
    tokens.push_back(token);
  }
  return tokens;
  }

std::vector<int> get_values_list(){
  std::string str = "";
    std::getline(std::cin, str);
    std::vector<std::string> nums_str = split(str,' ');
    std::vector<int> nums;
    for (std::string s : nums_str){
      nums.push_back(atoi(s.c_str()));
     }
     return nums;
}

void* producer_routine(void* arg) {
	std::vector<int> numlist =  get_values_list();  //get list of numbers
	Value* v =  static_cast<Value*>(arg);
	numbers = numlist.size();

	pthread_mutex_lock(&mutex2);
	while(ready_consumer == false)
			{
				pthread_cond_wait(&ready, &mutex2); // Wait for consumer to start
			}
	pthread_mutex_unlock(&mutex2);

	// Read data, loop through each value and update the value, notify consumer
	while (!(numlist.empty()))
	{
		pthread_mutex_lock(&mutex);
		while(update_value == true)		// wait until calculate sum
		{
			pthread_cond_wait(&cond2, &mutex); // Wait for consumer to process
		}
		(*v).update(numlist.back());
		numlist.pop_back();
		update_value = true;
		pthread_mutex_unlock(&mutex);
		pthread_cond_signal( &cond1);
	}
	pthread_mutex_lock(&mutex);
	finished_producer = true;
	pthread_mutex_unlock(&mutex);
	return nullptr;
  }

void* consumer_routine(void* arg) {
	pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
	pthread_t thId = pthread_self();

	pthread_mutex_lock(&mutex2);
	ready_consumer = true;			// notify about start
	pthread_mutex_unlock(&mutex2);
	pthread_cond_broadcast( &ready);

	Value* v =  static_cast<Value*>(arg);
	bool end = false;
	srand( time(NULL));
	while(true)
	{
		pthread_mutex_lock(&mutex3);
		if (iter_num != numbers)
		{
			pthread_mutex_lock(&mutex);
			while(update_value == false)		// wait until update data
			{
				pthread_cond_wait(&cond1, &mutex);
			}
			sum+=(*v).get();					// for every update issued by producer, read the value and add to sum
			update_value = false;
			pthread_mutex_unlock(&mutex);
			pthread_cond_signal( &cond2);
			iter_num++;
			pthread_mutex_unlock(&mutex3);
			usleep((rand() % sleep_time +1 )*1000);
		}
		else{
			pthread_mutex_unlock(&mutex3);
			break;
		}
	}

	void* ptr = &sum;
	int *intPtr = static_cast<int*>(ptr);
	return ptr;			// return pointer to result (aggregated result for all consumers)
}

void* consumer_interruptor_routine(void* arg) {

	pthread_mutex_lock(&mutex2);
		while(ready_consumer == false)
				{
					pthread_cond_wait(&ready, &mutex2); // Wait for consumer to start
				}
	pthread_mutex_unlock(&mutex2);

  // interrupt consumer while producer is running
	bool end = false;
	pthread_t* consumers = static_cast<pthread_t*>(arg);
	while (end == false)
	{
		int id = rand() % threads_num;
		pthread_cancel(consumers[id]);

		pthread_mutex_lock(&mutex);
		end = finished_producer;
		pthread_mutex_unlock(&mutex);
	}
	return nullptr;
}

int run_threads() {
	pthread_t producer;
	pthread_t interruptor;
	Value val ;
	void* result;

	pthread_create(&producer, NULL, producer_routine, &val);
  // start N threads and wait until they're done
	pthread_t* consumers = (pthread_t*) malloc(threads_num * sizeof(pthread_t));
	for (int i = 0; i < threads_num; ++i)
	{
	        pthread_create(&(consumers[i]), NULL, consumer_routine, &val);
	}
	pthread_create(&interruptor, NULL, consumer_interruptor_routine, consumers);


	for (int i = 0; i < threads_num; ++i)
	{
	      pthread_join(consumers[i], &result);
	      int* res =  static_cast<int*>(result);
	}
	pthread_join(producer, NULL);
	pthread_join(interruptor, NULL);
	int* res =  static_cast<int*>(result);
        std::free(consumers);

	pthread_mutex_destroy(&mutex);
	pthread_mutex_destroy(&mutex2);
	pthread_mutex_destroy(&mutex3);
	pthread_cond_destroy(&cond1);
	pthread_cond_destroy(&cond2);
	pthread_cond_destroy(&ready);
  // return aggregated sum of values

  return *res;
}

int main(int argc, char *argv[]) {
	if(argc < 3){
	    cout << "ERROR: must specify two arguments (N - number of consumers, T - max sleeping time)" << endl;
	    return 1;
	  }
	threads_num = atoi(argv[1]);
	sleep_time = atoi(argv[2]);
	cout << run_threads() << endl; // prints
	return 0;
}
