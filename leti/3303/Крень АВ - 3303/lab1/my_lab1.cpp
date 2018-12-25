#include <iostream>
#include <pthread.h>
#include <string>
#include <sstream>
#include <unistd.h>
#include <vector>

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

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t condStartConsumer = PTHREAD_COND_INITIALIZER;
pthread_cond_t condReadyConsumer = PTHREAD_COND_INITIALIZER;
pthread_cond_t condReadyValue = PTHREAD_COND_INITIALIZER;

volatile bool start_consumer = false;
volatile bool isReadyConsumer = false;
volatile int countReady = 0;
volatile bool isOver = false;

unsigned long numConsumers;
unsigned int sleepTime;
int count_producer_update = 0;

void* producer_routine(void* arg) {
    Value* value = reinterpret_cast<Value*>(arg);
    // Wait for consumer to start

    pthread_mutex_lock(&mutex);
    while(!start_consumer) 
        pthread_cond_wait(&condStartConsumer, &mutex);
    pthread_mutex_unlock(&mutex);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    //Read data
    string line;
    getline(cin, line);
    istringstream stream(line);
    //loop through each value and update the value
    int number;

    while (1) {
        if (!(stream >> number)) {
            isOver = true;
            break;
        }
        value->update(number);
        //notify consumer
        isReadyConsumer = false;
        count_producer_update++;
        pthread_cond_broadcast(&condReadyValue);

        //wait for consumer to process
        pthread_mutex_lock(&mutex);
        while(!isReadyConsumer)
            pthread_cond_wait(&condReadyConsumer, &mutex);
        pthread_mutex_unlock(&mutex);
    }
    pthread_cond_broadcast(&condReadyValue);
    return nullptr;

}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    Value* value = reinterpret_cast<Value*>(arg);

    // notify about start
    pthread_mutex_lock(&mutex);
    countReady++;
    pthread_mutex_unlock(&mutex);
    if (countReady == numConsumers) {
        start_consumer = true;
        pthread_cond_broadcast(&condStartConsumer);
        countReady = 0;
    }
    int count_consumer_update = 0;
    // for every update issued by producer, read the value and add to sum
    Value * sum = new Value();

    while (!isOver){
        pthread_mutex_lock(&mutex);
        while((count_consumer_update == count_producer_update) && !isOver)
            pthread_cond_wait(&condReadyValue, &mutex);
        pthread_mutex_unlock(&mutex);
        if (isOver) break;
        sum->update(sum->get() + value->get());
        count_consumer_update++;
        countReady++;
        if (countReady == numConsumers) {
            countReady = 0;
            isReadyConsumer = true;
            pthread_cond_signal(&condReadyConsumer);
        }
        usleep((rand() % (sleepTime + 1)) * 1000);

    }

    // return pointer to result (aggregated result for all consumers)
    return reinterpret_cast<void*>(sum);
}

void* consumer_interruptor_routine(void* arg) {
    vector<pthread_t*>* consumers = reinterpret_cast<vector<pthread_t*>*>(arg);

    pthread_mutex_lock(&mutex);
    while(!start_consumer) 
        pthread_cond_wait(&condStartConsumer, &mutex);
    pthread_mutex_unlock(&mutex);

    while(!isOver){
        unsigned long i = rand() % (consumers->size());
        pthread_cancel(*(consumers->at(i)));
    }
    return nullptr;
}

int run_threads() {
    srand(time(NULL));

    Value* value = new Value();

    pthread_t producer;
    pthread_create(&producer, nullptr, producer_routine, value);

    vector<pthread_t> consumers(numConsumers);
    for (int i = 0; i < numConsumers; i++)
        pthread_create(&consumers[i], nullptr, consumer_routine, value);

    pthread_t interruptor;
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    void* sum;
    pthread_join(consumers[0], &sum);
    for (int i = 1; i < numConsumers; i++)
        pthread_join(consumers[i], nullptr);

    return (reinterpret_cast<Value*>(sum))->get();
}

int main(int argc, const char *argv[]) {
    if (argc != 3) {
        cout << "Incorrect of arguments";
        return 1;
    }
    numConsumers = atoi(argv[1]);
    sleepTime = atoi(argv[2]);
    std::cout <<std::to_string(run_threads()) << std::endl;
    return 0;
}