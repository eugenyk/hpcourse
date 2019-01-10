#include <iostream>
#include <pthread.h>
#include <sstream>
#include <unistd.h>
#include <random>

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

namespace {
    pthread_mutex_t valueMutex;
    pthread_cond_t readCondition;
    pthread_cond_t writeCondition;

    volatile bool shouldRead = false;
    volatile bool finished = false;

    unsigned maxSleepTime = 0;

    int totalSum = 0;

    pthread_mutex_t consumersMutex;
    pthread_t* consumers;
    volatile unsigned consumersStartedCount = 0;
}

void* producer_routine(void* arg)
{
    Value& value = *static_cast<Value*>(arg);

    string inputLine;
    getline(cin, inputLine);
    istringstream iss(inputLine);

    do {
        int newValue = 0;
        iss >> newValue;

        pthread_mutex_lock(&valueMutex);
        while (shouldRead)
            pthread_cond_wait(&writeCondition, &valueMutex);

        value.update(newValue);
        finished = iss.eof();
        shouldRead = true;
        pthread_mutex_unlock(&valueMutex);
        pthread_cond_signal(&readCondition);
    } while (!finished);

    pthread_cond_broadcast(&readCondition);
    
    pthread_exit(nullptr);
}

void* consumer_routine(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    pthread_mutex_lock(&consumersMutex);
    consumers[consumersStartedCount++] = pthread_self();
    pthread_mutex_unlock(&consumersMutex);

    const auto& value = *static_cast<Value*>(arg);

    auto getSleepTime = []() -> useconds_t {
        static random_device device;
        static uniform_int_distribution<useconds_t> interval(0, maxSleepTime);

        return interval(device);
    };

    while (true) {
        pthread_mutex_lock(&valueMutex);
        while (!shouldRead && !finished)
            pthread_cond_wait(&readCondition, &valueMutex);

        if (!shouldRead) {
            pthread_mutex_unlock(&valueMutex);
            pthread_exit(&totalSum);
        }

        totalSum += value.get();

        shouldRead = false;
        pthread_mutex_unlock(&valueMutex);
        pthread_cond_signal(&writeCondition);

        usleep(getSleepTime());
    }
}

void* consumer_interruptor_routine(void* arg)
{
    static_cast<void>(arg);

    while (!finished) {
        static random_device random;

        pthread_mutex_lock(&consumersMutex);
        if (consumersStartedCount > 0) {
            uniform_int_distribution<unsigned long> index(0, consumersStartedCount - 1);
            pthread_cancel(consumers[index(random)]);
        }
        pthread_mutex_unlock(&consumersMutex);
        
        usleep(1);
    }

    pthread_exit(nullptr);
}

int run_threads(int consumersCount)
{
    pthread_mutex_init(&valueMutex, nullptr);
    pthread_cond_init(&readCondition, nullptr);
    pthread_cond_init(&writeCondition, nullptr);

    pthread_mutex_init(&consumersMutex, nullptr);

    Value value;

    pthread_t producer;
    pthread_t interruptor;
    consumers = new pthread_t[consumersCount];

    pthread_create(&producer, nullptr, &producer_routine, &value);
    pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, nullptr);
    for (int i = 0; i < consumersCount; ++i)
        pthread_create(&consumers[i], nullptr, &consumer_routine, &value);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);
    void* consumerRetval = nullptr;
    for (int i = 0; i < consumersCount; ++i)
        pthread_join(consumers[i], &consumerRetval);

    pthread_mutex_destroy(&valueMutex);
    pthread_cond_destroy(&readCondition);
    pthread_cond_destroy(&writeCondition);

    pthread_mutex_destroy(&consumersMutex);

    return *static_cast<int*>(consumerRetval);
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        cerr << "There must be 2 cmd-line arguments." << endl;
        return -1;
    }

    int consumersCount = atoi(argv[1]);
    maxSleepTime = static_cast<unsigned>(atoi(argv[2]));

    if (consumersCount <= 0) {
        cerr << "Wrong 1-st argument. Consumers count must be greather than zero." << endl;
        return -2;
    }

    cout << run_threads(consumersCount) << endl;
    return 0;
}
