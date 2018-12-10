#include <pthread.h>
#include <iostream>
#include <string>
#include <sstream>
#include <ctime>
#include <vector>
#include <unistd.h>

using namespace std;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
unsigned int sleepLim;
unsigned int countConsumers;
pthread_cond_t condConsumerStart = PTHREAD_COND_INITIALIZER;
bool isConsumerStart = false;
pthread_cond_t condValueChange = PTHREAD_COND_INITIALIZER;
bool isDone = false;
int countProducerUpdate = 0;

bool isAllConsumerUpdated = false;
pthread_cond_t condConsumerUpdated = PTHREAD_COND_INITIALIZER;

int countConsumersUpdated = 0;

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

    Value* sharedValue = (Value*) arg;
  // Wait for consumer to start
    pthread_mutex_lock(&lock);
    while(!isConsumerStart) pthread_cond_wait(&condConsumerStart, &lock);
    pthread_mutex_unlock(&lock);
    // Read data
    string input;
    getline(cin, input);
    stringstream inputStream;
    inputStream << input;
    // Loop through each value and update the value
    while(!isDone)
    {
        int curr_val;
        if (inputStream >> curr_val)
        {
            sharedValue->update(curr_val);
            countProducerUpdate++;
            // Notify consumer
            isAllConsumerUpdated = false;
            pthread_cond_broadcast(&condValueChange);
            //Wait for consumer to process
            pthread_mutex_lock(&lock);
            while(!isAllConsumerUpdated) pthread_cond_wait(&condConsumerUpdated, &lock);
            pthread_mutex_unlock(&lock);
        } else {
            isDone = true;
        }
    }
    pthread_cond_broadcast(&condValueChange);
    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    Value* sharedValue = (Value*) arg;
  // notify about start
     pthread_mutex_lock(&lock);
     countConsumersUpdated++;
     pthread_mutex_unlock(&lock);
     if (countConsumersUpdated == countConsumers)
     {
         isConsumerStart = true;
         pthread_cond_broadcast(&condConsumerStart);
         countConsumersUpdated = 0;
     }

  // for every update issued by producer, read the value and add to sum
     Value* result = new Value();
     int countValueUpdate = 0;
     while(!isDone)
     {
         pthread_mutex_lock(&lock);
         while(!isDone && (countValueUpdate == countProducerUpdate)) pthread_cond_wait(&condValueChange, &lock);
         pthread_mutex_unlock(&lock);
         if (isDone)
         {
             break;
         }
         result->update(result->get() + sharedValue->get());
         pthread_mutex_lock(&lock);
         countConsumersUpdated++;
         pthread_mutex_unlock(&lock);
         countValueUpdate++;
         if (countConsumersUpdated == countConsumers)
         {
             isAllConsumerUpdated = true;
             countConsumersUpdated = 0;
             pthread_cond_signal(&condConsumerUpdated);
         }
         usleep(rand() % (sleepLim*1000 + 1));
     }
    // return pointer to result (aggregated result for all consumers)
    return (void*) result;

}

void* consumer_interruptor_routine(void* arg) {
    vector<pthread_t*>* vectConsumers = (vector<pthread_t*>*) arg;
  // wait for consumer to start
    pthread_mutex_lock(&lock);
    while(!isConsumerStart) pthread_cond_wait(&condConsumerStart, &lock);
    pthread_mutex_unlock(&lock);
  // interrupt consumer while producer is running
    unsigned long randIDConsumers;
    while(!isDone)
    {
        randIDConsumers = rand()%vectConsumers->size();
        pthread_cancel(*vectConsumers->at(randIDConsumers));
    }
    return nullptr;
}

int run_threads() {
  // start N threads and wait until they're done
    Value* value = new Value();

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value);

    vector<pthread_t> consumers(countConsumers);
    for (int i = 0; i < countConsumers; i++)
        pthread_create(&consumers[i], nullptr, consumer_routine, value);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    void* result;
    pthread_join(consumers[0], &result);
    for (int i = 1; i < countConsumers; i++)
        pthread_join(consumers[i], nullptr);
    // return aggregated sum of values
    return ((Value*)result)->get();

}

int main(int argc, const char *argv[]) {
    if (argc != 3)
    {
        std::cout << "Wrong parameters!";
        return 1;
    }

    countConsumers = atoi(argv[1]);
    sleepLim = atoi(argv[2]);
    cout << run_threads() << endl;

    return 0;
}