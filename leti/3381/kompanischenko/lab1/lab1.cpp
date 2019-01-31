#include <iostream>
#include <pthread.h>
#include <sstream>
#include <unistd.h>
#include <vector>

    pthread_barrier_t waitForStartAllThreads;
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    volatile bool value_updated = false;
    volatile bool finish = false;

    unsigned maxSleepTime = 0;

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

void* producer_routine(void* arg)
{
    Value& value = *static_cast<Value*>(arg);
    pthread_barrier_wait(&waitForStartAllThreads);

    std::string inputLine;
    std::getline(std::cin, inputLine);
    std::istringstream iss(inputLine);

    do {
        int newValue = 0;
        iss >> newValue;
        pthread_mutex_lock(&mutex);
        while (value_updated)
            pthread_cond_wait(&cond, &mutex);

        value.update(newValue);
        finish = iss.eof();
        value_updated = true;
        pthread_mutex_unlock(&mutex);
        pthread_cond_signal(&cond);
    } while (!finish);

    pthread_cond_broadcast(&cond);
    pthread_exit(nullptr);
}

void* consumer_routine(void* arg)
{
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);
    const auto& value = *static_cast<Value*>(arg);
    static int* sum = new int { 0 };
    auto getSleepTime = []() -> useconds_t {
        return maxSleepTime ? static_cast<useconds_t>(rand()) % maxSleepTime : 0;
    };
    pthread_barrier_wait(&waitForStartAllThreads);

    do {
        pthread_mutex_lock(&mutex);
        while (!value_updated && !finish)
            pthread_cond_wait(&cond, &mutex);

        if (!value_updated) {
            pthread_mutex_unlock(&mutex);
            pthread_exit(sum);
        }

        (*sum) += value.get();

        value_updated = false;
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&cond);
        usleep(getSleepTime());
    } while (true);
}

void* consumer_interruptor_routine(void* arg)
{
    const auto& consumers = *static_cast<std::vector<pthread_t>*>(arg);
    pthread_barrier_wait(&waitForStartAllThreads);

    while (!finish) {
        unsigned long index = static_cast<unsigned long>(random()) % consumers.size();
        pthread_cancel(consumers[index]);
        usleep(1);
    }

    pthread_exit(nullptr);
}

int run_threads(int consummersCount)
{
    pthread_barrier_init(&waitForStartAllThreads, nullptr, static_cast<unsigned>(consummersCount + 2));
    pthread_mutex_init(&mutex, nullptr);
    pthread_cond_init(&cond, nullptr);

    Value value;

    pthread_t producer;
    pthread_t interaptor;
    std::vector<pthread_t> consumers(static_cast<unsigned long>(consummersCount));

    pthread_create(&producer, nullptr, &producer_routine, &value);
    pthread_create(&interaptor, nullptr, &consumer_interruptor_routine, &consumers);
    for (pthread_t& consumer : consumers)
        pthread_create(&consumer, nullptr, &consumer_routine, &value);

    pthread_join(producer, nullptr);
    pthread_join(interaptor, nullptr);
    void* consumerRetval = nullptr;
    for (const pthread_t& consumer : consumers)
        pthread_join(consumer, &consumerRetval);

    int sum = 0;
    if (nullptr != consumerRetval) {
        int* consumerRetvalInt = static_cast<int*>(consumerRetval);
        sum = *consumerRetvalInt;
        delete consumerRetvalInt;
    }

    return sum;
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        return -1;
    }

    int consumersCount = atoi(argv[1]);
    maxSleepTime = static_cast<unsigned>(atoi(argv[2]));

    if (consumersCount <= 0) {
        return -2;
    }

    std::cout << run_threads(consumersCount) << std::endl;
    return 0;
}