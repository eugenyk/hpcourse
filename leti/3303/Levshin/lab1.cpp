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
volatile bool isStartConsumer = false;

pthread_cond_t condReadyConsumer = PTHREAD_COND_INITIALIZER;
volatile bool isReadyConsumer = false;
volatile int countReady = 0;

pthread_cond_t condReadyValue = PTHREAD_COND_INITIALIZER;
volatile bool isEnd = false;

unsigned int sleepLimit;
unsigned long countConsumers;
int count_producer_update = 0;

void* producer_routine(void* arg) {
    Value* value = reinterpret_cast<Value*>(arg);
    // Wait for consumer to start

    pthread_mutex_lock(&mutex);
    while(!isStartConsumer) pthread_cond_wait(&condStartConsumer, &mutex);
    pthread_mutex_unlock(&mutex);

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    //Read data
    string input;
    cout << "Enter numbers:\n\t";
    getline(cin, input);
    stringstream stream(input);
    //loop through each value and update the value
    int number;

    while (true) {
        if (!(stream >> number)) {
            isEnd = true;
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
    if (countReady == countConsumers) {
        isStartConsumer = true;
        pthread_cond_broadcast(&condStartConsumer);
        countReady = 0;
    }
    int count_consumer_update = 0;
    // for every update issued by producer, read the value and add to sum
    Value * sum = new Value();


    while (!isEnd){
        pthread_mutex_lock(&mutex);
        while((count_consumer_update == count_producer_update) && !isEnd) {
            pthread_cond_wait(&condReadyValue, &mutex);
        }
        pthread_mutex_unlock(&mutex);
        if (isEnd) break;
        sum->update(sum->get() + value->get());
        count_consumer_update++;
        countReady++;
        if (countReady == countConsumers) {
            countReady = 0;
            isReadyConsumer = true;
            pthread_cond_signal(&condReadyConsumer);
        }
        usleep((rand() % (sleepLimit + 1))*1000);

    }

    // return pointer to result (aggregated result for all consumers)
    return reinterpret_cast<void*>(sum);
}

void* consumer_interruptor_routine(void* arg) {
    vector<pthread_t*>* consumers = reinterpret_cast<vector<pthread_t*>*>(arg);

    pthread_mutex_lock(&mutex);
    while(!isStartConsumer) pthread_cond_wait(&condStartConsumer, &mutex);
    pthread_mutex_unlock(&mutex);

    while(!isEnd){
        unsigned long i = rand() % (consumers->size());
        pthread_cancel(*(consumers->at(i)));
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
    void* sum;
    pthread_join(consumers[0], &sum);
    for (int i = 1; i < countConsumers; i++)
        pthread_join(consumers[i], nullptr);
    // return aggregated sum of values
    return (reinterpret_cast<Value*>(sum))->get();
}

int main(int argc, const char *argv[]) {
    srand(time(NULL));
    if (argc != 3) {
        cout << "Parameter error:\n\t1st argument - number of consumer-thread\n\t2st argument - max sleep limit\n";
        return 1;
    }
    countConsumers = atoi(argv[1]);
    sleepLimit = atoi(argv[2]);
    std::cout << "Result:\n\t" + std::to_string(run_threads()) << std::endl;
    return 0;
}
