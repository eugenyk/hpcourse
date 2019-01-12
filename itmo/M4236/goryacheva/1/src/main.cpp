#include <iostream>
#include <pthread.h>
#include <vector>
#include <unistd.h>

#define NOT_STARTED 0
#define STARTED 1
#define PRODUCER_READY 2
#define CONSUMER_READY 3
#define FINISHED 4

static int MAX_SLEEP_TIME;
static size_t CONSUMER_NUMBER;

int status = NOT_STARTED;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_monitor = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_monitor = PTHREAD_COND_INITIALIZER;

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
    // Wait for consumer to start
    if(status == NOT_STARTED) {
        pthread_mutex_lock(&mutex);
        while (status != STARTED) {
            pthread_cond_wait(&producer_monitor, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process
    auto value = static_cast<Value *>(arg);

    pthread_mutex_lock(&mutex);
    int num;
    while (std::cin >> num) {
        value->update(num);
        status = PRODUCER_READY;
        pthread_cond_signal(&consumer_monitor);
        while (status != CONSUMER_READY) {
            pthread_cond_wait(&producer_monitor, &mutex);
        }
    }

    status = FINISHED;
    pthread_cond_broadcast(&consumer_monitor);
    pthread_mutex_unlock(&mutex);

    return static_cast<void *>(value);
}

void* consumer_routine(void* arg) {
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    // notify about start
    if(status == NOT_STARTED) {
        pthread_mutex_lock(&mutex);
        status = STARTED;
        pthread_cond_broadcast(&producer_monitor);
        pthread_mutex_unlock(&mutex);
    }

    // for every update issued by producer, read the value and add to sum
    // return pointer to result (aggregated result for all consumers)
    auto value = static_cast<Value *>(arg);
    int sum = 0;
    while (status != FINISHED) {
        pthread_mutex_lock(&mutex);
        while (status != FINISHED && status != PRODUCER_READY) {
            pthread_cond_wait(&consumer_monitor, &mutex);
        }
        if(status == FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        sum += value->get();
        status = CONSUMER_READY;
        pthread_mutex_unlock(&mutex);
        pthread_cond_broadcast(&producer_monitor);

        int time = rand() % MAX_SLEEP_TIME;
        usleep(static_cast<__useconds_t>(time * 1000));
    }

    return new int(sum);
}

void* consumer_interruptor_routine(void* arg) {
    auto consumers = static_cast<const std::vector<pthread_t> *>(arg);

    // wait for consumer to start
    if(status == NOT_STARTED) {
        pthread_mutex_lock(&mutex);
        while (status != STARTED) {
            pthread_cond_wait(&producer_monitor, &mutex);
        }
        pthread_mutex_unlock(&mutex);
    }
//     interrupt consumer while producer is running
    while (status != FINISHED) {
        pthread_mutex_lock(&mutex);
        while (status != CONSUMER_READY && status != FINISHED ) {
            pthread_cond_wait(&producer_monitor, &mutex);
        }
        if(status == FINISHED) {
            pthread_mutex_unlock(&mutex);
            break;
        }
        auto i = static_cast<size_t>(rand() % CONSUMER_NUMBER);
        pthread_cancel(reinterpret_cast<pthread_t>(consumers->at(i)));
        pthread_mutex_unlock(&mutex);
    }
    return nullptr;
}

int run_threads() {
    // start N threads and wait until they're done
    pthread_t producer;
    pthread_t interruptor;
    std::vector<pthread_t> consumers(CONSUMER_NUMBER);
    auto * value = new Value();

    pthread_create(&producer, nullptr, producer_routine, value);
    for (auto& consumer: consumers) {
        pthread_create(&consumer, nullptr, consumer_routine, value);
    }
    pthread_create(&interruptor, nullptr, consumer_interruptor_routine, &consumers);

    pthread_join(producer, nullptr);
    pthread_join(interruptor, nullptr);

    // return aggregated sum of values
    int result = 0;
    for (auto& consumer: consumers) {
        int* res;
        pthread_join(consumer, reinterpret_cast<void**>(&res));
        result += (*res);
        delete res;
    }

    pthread_cond_destroy(&consumer_monitor);
    pthread_cond_destroy(&producer_monitor);
    pthread_mutex_destroy(&mutex);
    return result;
}

void init(int argc, char* argv[]) {
    if(argc != 3) {
        std::cout << "Incorrect arguments number!" << std::endl;
        exit(1);
    }

    CONSUMER_NUMBER = static_cast<size_t>(std::atoi(argv[1]));
    MAX_SLEEP_TIME = std::atoi(argv[2]);

    if( CONSUMER_NUMBER <= 0 || MAX_SLEEP_TIME <= 0) {
        std::cout << "Incorrect arguments!" << std::endl;
        exit(1);
    }
}

int main(int argc, char* argv[]) {
    init(argc, argv);
    std::cout << run_threads() << std::endl;
    return 0;
}