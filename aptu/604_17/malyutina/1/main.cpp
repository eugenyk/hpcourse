#include <pthread.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <iterator>

class Value {
public:
    Value() : _value(0) {}

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

pthread_t producer;
pthread_t consumer;
pthread_t interruptor;

pthread_cond_t value_update;
pthread_cond_t end_consumer;

pthread_barrier_t barrier;

bool update = false;
bool start_consumer = true;
bool producer_updated_finished = false;

size_t number_of_ints = 0;


void *producer_routine(void *arg) {
    // Wait for consumer to start

    // Read data, loop through each value and update the value, notify consumer, wait for consumer to process

    Value *value = reinterpret_cast<Value *>(arg);

    int data;
    std::string line;

    getline(std::cin, line);

    std::istringstream is(line);
    std::vector<int> input;

    while (is >> data) {
        input.emplace_back(data);
    }

    number_of_ints = input.size();

    pthread_barrier_wait(&barrier);

    for (auto next_int : input) {
        pthread_mutex_lock(&mutex);

        value->update(next_int);
        update = true;
        start_consumer = false;

        pthread_cond_signal(&value_update);

        while (!start_consumer) {
            pthread_cond_wait(&end_consumer, &mutex);
        }

        pthread_mutex_unlock(&mutex);
    }
    producer_updated_finished = true;

    return nullptr;
}

void *consumer_routine(void *arg) {
    // notify about start
    // allocate value for result
    // for every update issued by producer, read the value and add to sum
    // return pointer to result
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, nullptr);

    Value *value = reinterpret_cast<Value *>(arg);

    int *sum = new int;
    *sum = 0;
    pthread_barrier_wait(&barrier);

    for (size_t i = 0; i < number_of_ints; ++i) {
        pthread_mutex_lock(&mutex);

        while (!update) {
            pthread_cond_wait(&value_update, &mutex);
        }

        *sum += value->get();
        start_consumer = true;
        update = false;

        pthread_cond_signal(&end_consumer);
        pthread_mutex_unlock(&mutex);
    }
    return reinterpret_cast<void *>(sum);
}

void *consumer_interruptor_routine(void *arg) {
    // wait for consumer to start

    // interrupt consumer while producer is running

    pthread_barrier_wait(&barrier);

    while (!producer_updated_finished) {
        pthread_cancel(consumer);
    }

    return nullptr;
}

int run_threads() {
    // start 3 threads and wait until they're done
    // return sum of update values seen by consumer
    pthread_cond_init(&end_consumer, nullptr);
    pthread_cond_init(&value_update, nullptr);

    pthread_barrier_init(&barrier, nullptr, 3);

    Value *value = new Value();

    pthread_create(&consumer, nullptr, &consumer_routine, value);
    pthread_create(&producer, nullptr, &producer_routine, value);
    pthread_create(&interruptor, nullptr, &consumer_interruptor_routine, value);

    int *res = new int;

    pthread_join(producer, nullptr);
    pthread_join(consumer, (void **) &res);
    pthread_join(interruptor, nullptr);

    delete value;

    int result = *res;
    delete res;

    return result;
}

int main() {
    std::cout << run_threads() << std::endl;
    return 0;
}