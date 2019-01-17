#include <iostream>
#include <random>
#include <sstream>
#include <ctime>
#include <unistd.h>

using namespace std;

unsigned int sleepLim;
unsigned int countConsumers;

pthread_barrier_t bThreadsReady;
pthread_barrier_t bValueReady;
pthread_barrier_t bConsumersUpdatedValue;

bool isDone = false;

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
    auto sharedValue = (Value*) arg;

    // Wait for consumer to start
    pthread_barrier_wait(&bThreadsReady);

    // Read data
    string input;
    getline(cin, input);
    stringstream inputStream;
    inputStream << input;

    // Loop through each value and update the value
    int curr_val;
    while(!isDone)
    {
        if (inputStream >> curr_val)
        {
            sharedValue->update(curr_val);
            // Notify consumer
            pthread_barrier_wait(&bValueReady);
            //Wait for consumer to process
            pthread_barrier_wait(&bConsumersUpdatedValue);

        } else {
            isDone = true;
        }
    }
    pthread_barrier_wait(&bValueReady);
    return nullptr;
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    random_device dev;
    mt19937 gen(dev());
    uniform_int_distribution<uint> uid(0, (uint)sleepLim);
    auto sharedValue = (Value*) arg;

    // notify about start
    pthread_barrier_wait(&bThreadsReady);

    // for every update issued by producer, read the value and add to sum
    auto result = new Value();
    while(true)
    {
        pthread_barrier_wait(&bValueReady);
        if (isDone)
        {
            break;
        }
        result->update(result->get() + sharedValue->get());
        usleep(uid(gen));
        pthread_barrier_wait(&bConsumersUpdatedValue);
    }
    // return pointer to result (aggregated result for all consumers)
    return (void*)result;

}

void* consumer_interruptor_routine(void* arg) {
    auto vectConsumers = (vector<pthread_t>*) arg;
    random_device dev;
    mt19937 gen(dev());
    uniform_int_distribution<ulong> uid(0, vectConsumers->size() - 1);

    // wait for consumer to start
    pthread_barrier_wait(&bThreadsReady);

    // interrupt consumer while producer is running
    unsigned long randIDConsumers;
    while(!isDone)
    {
        randIDConsumers = uid(gen);
        pthread_cancel(vectConsumers->at(randIDConsumers));
    }
    return nullptr;
}

int run_threads() {

    //Init barriers
    pthread_barrier_init(&bThreadsReady, nullptr, countConsumers + 2);
    pthread_barrier_init(&bValueReady, nullptr, countConsumers + 1);
    pthread_barrier_init(&bConsumersUpdatedValue, nullptr, countConsumers + 1);

    // start N threads and wait until they're done
    auto value = new Value();

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
    auto sum = (Value *)result;
    int answer = sum->get();
    delete sum;
    for (int i = 1; i < countConsumers; i++) {
        pthread_join(consumers[i], &result);
        sum = (Value *)result;
        delete sum;
    }
    delete value;
    pthread_barrier_destroy(&bThreadsReady);
    pthread_barrier_destroy(&bValueReady);
    pthread_barrier_destroy(&bConsumersUpdatedValue);
    // return aggregated sum of values
    return answer;

}

int main(int argc, const char *argv[]) {
    if (argc != 3)
    {
        std::cout << "Wrong parameters!";
        return 1;
    }

    countConsumers = (uint)stoi(argv[1]);
    sleepLim = (uint)stoi(argv[2]);
    cout << run_threads() << endl;

    return 0;
}